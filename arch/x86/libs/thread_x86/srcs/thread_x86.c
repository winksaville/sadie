/*
 * copyright 2015 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#include <thread_x86.h>

#include <interrupts_x86.h>
#include <native_x86.h>

#include <ac_assert.h>
#include <ac_memmgr.h>
#include <ac_inttypes.h>

#define NDEBUG
#include <ac_debug_printf.h>

#define RESCHEDULE_ISR 0xfe

static void* entry_trampoline(void* param) __attribute__ ((noreturn));

typedef struct tcb_x86 {
  ac_u32 thread_id;
  struct tcb_x86* pnext_tcb;
  void*(*entry)(void*);
  void* entry_arg;
  ac_u8* pstack;
  ac_u8* sp;
  ac_u16 ss;
} tcb_x86;

typedef struct {
  ac_u32 max_count;
  tcb_x86 tcbs[];
} ac_threads;

#define DEFAULT_FLAGS 0

#define AC_THREAD_ID_EMPTY (ac_u32)-1
#define AC_THREAD_ID_STARTING (ac_u32)-2
#define AC_THREAD_ID_ZOMBIE (ac_u32)-3

/**
 * The idle thread
 */
static ac_u8 idle_stack[AC_THREAD_STACK_MIN] __attribute__ ((aligned(64)));
static tcb_x86 idle_tcb;

/**
 * The main thread
 */
static tcb_x86 main_tcb;

/**
 * A NON-EMPTY circular linked list of tcbs to run,
 * it always contains at least the idle tcb, if nohting else.
 */
static tcb_x86* pready;

/**
 * The list of threads which are waiting for some
 * type of event??
 */
static tcb_x86* pwaiting_list;

/**
 * All of the threads
 */
static ac_threads* pthreads;

/**
 * Initialize the stack frame assuming no stack switching
 */
static void init_stack_frame(ac_u8* tos, ac_uptr flags, void* (*entry)(void*),
    void* entry_arg, ac_u8** psp, ac_u16 *pss) {
  struct intr_frame* sf = (struct intr_frame*)(tos - sizeof(struct intr_frame));

  ac_static_assert(sizeof(void*) == sizeof(ac_uptr),
      "Assumption that void* is sizeof ac_uptr is false");
  sf->ip = (ac_uptr)entry;
  sf->cs = 0; // TODO: how to init code segment
  sf->flags = flags;

  sf->sp = (ac_uptr)(tos + sizeof(struct intr_frame));
  sf->ss = 0; // TODO: how to init stack segment

  *psp = (ac_u8*)sf->sp;
  *pss = sf->ss;
}


/**
 * A routine that calls the actual thread entry
 *
 * @param param is a pointer to the tcb_x86.
 *
 * @return AC_NULL
 */
static void* entry_trampoline(void* param) {
  // Invoke the entry point
  tcb_x86* ptcb = (tcb_x86*)param;
  ptcb->entry(ptcb->entry_arg);

  // Mark as zombie
  ac_u32* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_ZOMBIE, __ATOMIC_RELEASE);

  intr(RESCHEDULE_ISR);

  while (AC_TRUE);
}

/**
 * Remove any zombie threads recoverying the stack
 * and the tcb.
 */
static void remove_zombies(void) {
  tcb_x86* pzombie = AC_NULL;

  // Loop through the list of ready tcb removing
  // ZOMBIE entries. We'll start at idle_tcb as
  // the tail since its guaranteed to be on the
  // list and not a ZOMBIE.
  tcb_x86* ptail = &idle_tcb;
  tcb_x86* phead = idle_tcb.pnext_tcb;
  while (phead != &idle_tcb) {
    cli();
    if (phead->thread_id == AC_THREAD_ID_ZOMBIE) {
      // phead is a ZOMBIE, remove it from the
      // list advancing the head past it.
      pzombie = phead;
      phead = phead->pnext_tcb;
      ptail->pnext_tcb = phead;
    } else {
      // Not a ZOMBIE advance head and tail
      ptail = phead;
      phead = phead->pnext_tcb;
    }
    sti();

    if (pzombie != AC_NULL) {
      // Remove the ZOMBIE
      if (pzombie->pstack != AC_NULL) {
        ac_free(pzombie->pstack);
      }
      ac_u32* pthread_id = &pzombie->thread_id;
      __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
      pzombie = AC_NULL;
    }
  }
}

/**
 * Add an initialized tcb following pcur
 * * @param tcb to add to the ready list */
static void add_after(tcb_x86* pcur, tcb_x86* pnew) {
  cli();
  pnew->pnext_tcb = pcur->pnext_tcb;
  pcur->pnext_tcb = pnew;
  sti();
}

/**
 * Idle routine, invoked if no other thread is ready.
 * It must never exit.
 */
static void* idle(void* param) {
  remove_zombies();
  while (AC_TRUE) {
    // TODO: Goto a lower power state
  }
  return AC_NULL;
}

/**
 * Initialize tcb it with the entry and entry_arg.
 *
 * @param entry routine to call to start thread
 * @param entry_arg is the argument to pass to entry
 *
 * Return AC_NULL if an error, i.e. nono available
 */
static void tcb_init(tcb_x86* ptcb, ac_u32 thread_id, ac_u8* pstack,
    void*(*entry)(void*), void* entry_arg) {
  ptcb->entry = entry;
  ptcb->entry_arg = entry_arg;
  ptcb->pstack = pstack;
  ac_u32* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, thread_id, __ATOMIC_RELEASE);
}

/**
 * Get a tcb and initialize it with the entry and entry_arg.
 *
 * @param entry routine to call to start thread
 * @param entry_arg is the argument to pass to entry
 *
 * Return AC_NULL if an error, i.e. nono available
 */
static tcb_x86* get_tcb(void*(*entry)(void*), void* entry_arg) {
  // Find an tcb_x86
  tcb_x86* ptcb;
  ac_bool found = AC_FALSE;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    ptcb = &pthreads->tcbs[i];

    ac_u32 empty = AC_THREAD_ID_EMPTY;
    ac_u32* pthread_id = &ptcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_STARTING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      tcb_init(ptcb, i, AC_NULL, entry, entry_arg);
      found = AC_TRUE;
      break;
    }
  }

  if (!found) {
    ptcb = AC_NULL;
  }
  return ptcb;
}


/**
 * Create a thread with a stack, and the initial flags
 *
 * If stack_size is <= 0 a "default" stack size will be used.
 *
 * If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 * otherwise an error is returned and the thread is not created.
 *
 * Return 0 on success !0 if an error.
 */
static tcb_x86* thread_create(ac_size_t stack_size, ac_uptr flags,
    void*(*entry)(void*), void* entry_arg) {
  tcb_x86* ptcb = AC_NULL;
  ac_u8* pstack = AC_NULL;
  int error = 0;

  // Allocate a stack
  if (stack_size <= 0) {
    stack_size = AC_THREAD_STACK_MIN;
  }
  if (stack_size < AC_THREAD_STACK_MIN) {
    error = 1;
    goto done;
  }

  pstack = ac_malloc(stack_size);
  if (pstack == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_OOM
    goto done;
  }

  // Get the tcb and initialize the stack frame
  ptcb = get_tcb(entry, entry_arg);
  if (ptcb == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_TO_MANY_THREADS
    goto done;
  }
  ptcb->pstack = pstack;
  init_stack_frame(pstack + stack_size, flags, entry_trampoline, ptcb,
      &ptcb->sp, &ptcb->ss);

  // Add this after pready
  add_after(pready, ptcb);

done:
  if (error != 0) {
    if (pstack != AC_NULL) {
      ac_free(pstack);
    }
  }

  return ptcb;
}

/**
 * Next tcb
 */
static tcb_x86* next_tcb(tcb_x86* pcur_tcb) {
  // Next thread
  tcb_x86* pnext = pcur_tcb->pnext_tcb;

  // Skip any ZOMBIE threads it is ASSUMED the list
  // has at least one non ZOMBIE thread, the idle thread,
  // so this is guarranteed to not be an endless loop.
  ac_u32* pthread_id = &pnext->thread_id;
  ac_u32 thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  while(thread_id == AC_THREAD_ID_ZOMBIE) {
    // Skip the ZOMBIE
    pnext = pnext->pnext_tcb;
    pthread_id = &pnext->thread_id;
    thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  }

  return pnext;
}

/**
 * Thread scheduler, for internal only, ASSUMES interrupts are DISABLED!
 *
 * @param pcur_sp is the stack of the current thread
 * @return the stack of the next thread to run
 */
void thread_scheduler(ac_u8* sp, ac_u16 ss, ac_u8**psp, ac_u16* pss) {
  // Save the current thread stack pointer
  pready->sp = sp;
  pready->ss = ss;

  // Get the next tcb
  pready = next_tcb(pready);
  if (pready == &idle_tcb) {
    // Skip idle once, if idle is the only thread
    // then we'll execute this time.
    pready = next_tcb(pready);
  }

  *psp = pready->sp;
  *pss = pready->ss;
}

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) {
  // Setup the stack frame exected by ac_reschedule
  // and branch to ac_reschedule
  intr(RESCHEDULE_ISR);
}

/**
 * Reschedule the CPU to the next ready thread. CAREFUL, this is
 * invoked by ac_init/srcs/ac_exception_irq_wrapper.S as such the
 * stack frame pointed to by sp must be identical.
 *
 * Interrupts will be disabled as the scheduler will be invoked
 */
INTERRUPT_HANDLER
void reschedule_isr(struct intr_frame* frame) {
  ac_u8* stack_ptr;
  ac_u16 stack_seg;

  thread_scheduler((ac_u8*)get_sp(), get_ss(), &stack_ptr, &stack_seg);
  // Switch stacks
  __asm__ volatile (
    "mov %0, %%rsp\n"
    "mov %1, %%ss\n" :: "r"(stack_ptr), "r"(stack_seg)
  );
}


/**
 * Early initialization of this module
 */
void ac_thread_early_init() {
  // Initialize reschedule isr
  set_intr_handler(RESCHEDULE_ISR, reschedule_isr);

  // Initialize the idle_stack
  ac_uptr idle_flags = 0; //PSR_MODE_SYS | PSR_FIQ_DISABLED | PSR_IRQ_ENABLED;
  init_stack_frame(idle_stack + sizeof(idle_stack), idle_flags, idle, AC_NULL,
      &idle_tcb.sp, &idle_tcb.ss);

  // Get the main tcb, no stack is needed because its
  // the one currently being used.
  tcb_init(&main_tcb, 0, AC_NULL, AC_NULL, AC_NULL);
  tcb_init(&idle_tcb, 1, idle_stack, idle, AC_NULL);

  // Empty waiting list
  pwaiting_list = AC_NULL;

  // Add main as the only tcb on pready to start
  main_tcb.pnext_tcb = &main_tcb;
  pready = &main_tcb;

  // Add idle after that
  add_after(pready, &idle_tcb);

  // TODO: Start a periodic timer for scheduling
  // ac_start_periodic_timer(1000); // Timer for scheduling
}

/**
 * Initialize this module
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  // Create array of the threads
  ac_u32 size = sizeof(ac_threads) + (max_threads * sizeof(tcb_x86));
  pthreads = ac_malloc(size);
  ac_assert(pthreads != AC_NULL);

  pthreads->max_count = max_threads;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    pthreads->tcbs[i].thread_id = AC_THREAD_ID_EMPTY;
    pthreads->tcbs[i].entry = AC_NULL;
    pthreads->tcbs[i].entry_arg = AC_NULL;
    pthreads->tcbs[i].pnext_tcb = AC_NULL;
    pthreads->tcbs[i].pstack = AC_NULL;
    pthreads->tcbs[i].sp = AC_NULL;
  }
}

/**
 * Create a thread and i:qnvoke the entry_p:w
 * oint passing
 * the parameter entry_arg.
 *
 * @param stack_size is <= 0 a "default" stack size will be used.
 *        If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 *        otherwise an error is returned and the thread is not created.
 *
 * @return 0 on success !0 if an error.
 */
ac_u32 ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg) {
  tcb_x86* ptcb = thread_create(stack_size, DEFAULT_FLAGS, entry, entry_arg);
  return ptcb == AC_NULL ? 1 : 0;
}
