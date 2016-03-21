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

#include <ac_thread.h>

#include <ac_assert.h>
#include <ac_memmgr.h>
#include <ac_inttypes.h>
#include <ac_interrupts.h>

#include <_timer.h>

static void* entry_trampoline(void* param) __attribute__ ((noreturn));

void ac_reschedule(void) __attribute__ ((noreturn))
                               __attribute__ ((naked));

typedef struct ac_tcb {
  ac_u32 thread_id;
  struct ac_tcb* pnext_tcb;
  void*(*entry)(void*);
  void* entry_arg;
  ac_u8* pstack;
  ac_u8* sp;
} ac_tcb;

typedef struct {
  ac_u32 max_count;
  ac_tcb tcbs[];
} ac_threads;

#define AC_THREAD_ID_EMPTY (ac_u32)-1
#define AC_THREAD_ID_STARTING (ac_u32)-2
#define AC_THREAD_ID_ZOMBIE (ac_u32)-3

/**
 * The idle thread
 */
static ac_u8 idle_stack[AC_THREAD_STACK_MIN] __attribute__ ((aligned(64)));
static ac_tcb idle_tcb;

/**
 * The main thread
 */
static ac_tcb main_tcb;

/**
 * A NON-EMPTY circular linked list of tcbs to run,
 * it always contains at least the idle tcb, if nohting else.
 */
static ac_tcb* pready;

/**
 * The list of threads which are waiting for some
 * type of event??
 */
static ac_tcb* pwaiting_list;

/**
 * All of the threads
 */
static ac_threads* pthreads;

/**
 * ARMv6 PSR mode
 */
#define PSR_MODE_USR  0x10
#define PSR_MODE_FIQ  0x11
#define PSR_MODE_IRQ  0x12
#define PSR_MODE_SVC  0x13
#define PSR_MODE_ABT  0x17
#define PSR_MODE_UDEF 0x1B
#define PSR_MODE_SYS  0x1F

#define PSR_IRQ_ENABLED  0x00
#define PSR_IRQ_DISABLED 0x80
#define PSR_FIQ_ENABLED  0x00
#define PSR_FIQ_DISABLED 0x40

/**
 * This structure matches the stack layout as expected
 * by ac_exception_irq_wrapper.S.
 *
 * Because ac_malloc always aligns on a 8 byte boundry
 * there is a padding work and the align_factor is 4.
 */
#pragma pack(push,4)
typedef struct {
  ac_u32 align_factor;      // A 4 as stack needed to be aligned
  ac_u32 lr;                // Extra align
  ac_u32 padding;           // Padding to align stack on 8 byte boundry
  ac_u32 r0;                // R0
  ac_u32 r1;                // R1
  ac_u32 r2;                // R2
  ac_u32 r3;                // R3
  ac_u32 r12;               // R12
  ac_u32 pc;                // Program Counter
  ac_u32 psr;               // Program Status word
} irq_wrapper_stack;
#pragma pack(pop)

static ac_u8* init_stack_frame(ac_u8* tos, ac_u32 psr, void* (*entry)(void*),
    void* entry_arg) {
  irq_wrapper_stack* sp = (irq_wrapper_stack*)(tos - sizeof(irq_wrapper_stack));

  ac_static_assert(sizeof(void*) == sizeof(ac_u32),
      "Assumption that void* is 32 bits if false");
  sp->align_factor = 4;
  sp->lr = (ac_u32)entry;
  sp->padding = 0;
  sp->r0 = (ac_u32)entry_arg;
  sp->r1 = 0;
  sp->r2 = 0;
  sp->r3 = 0;
  sp->r12 = 0;
  sp->pc = (ac_u32)entry;
  sp->psr = psr;

  return (ac_u8*)sp;
}

/**
 * A routine that calls the actual thread entry
 *
 * @param param is a pointer to the ac_tcb.
 *
 * @return AC_NULL
 */
static void* entry_trampoline(void* param) {
  // Invoke the entry point
  ac_tcb* ptcb = (ac_tcb*)param;
  ptcb->entry(ptcb->entry_arg);

  // Mark as zombie
  ac_u32* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_ZOMBIE, __ATOMIC_RELEASE);
  
  ac_reschedule();
}

/**
 * Remove any zombie threads recoverying the stack
 * and the tcb.
 */
static void remove_zombies(void) {
  ac_tcb* pzombie = AC_NULL;

  // Loop through the list of ready tcb removing
  // ZOMBIE entries. We'll start at idle_tcb as
  // the tail since its guaranteed to be on the
  // list and not a ZOMBIE.
  ac_tcb* ptail = &idle_tcb;
  ac_tcb* phead = idle_tcb.pnext_tcb;
  while (phead != &idle_tcb) {
    ac_interrupts_cpu_disable();
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
    ac_interrupts_cpu_enable();

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
static void add_after(ac_tcb* pcur, ac_tcb* pnew) {
  ac_interrupts_cpu_disable();
  pnew->pnext_tcb = pcur->pnext_tcb;
  pcur->pnext_tcb = pnew;
  ac_interrupts_cpu_enable();
}

/**
 * Idle routine, invoked if no other thread is ready.
 * It must never exit.
 */
#define IDLE_STACK_SIZE AC_THREAD_STACK_MIN
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
static void tcb_init(ac_tcb* ptcb, ac_u32 thread_id, ac_u8* pstack,
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
static ac_tcb* get_tcb(void*(*entry)(void*), void* entry_arg) {
  // Find an ac_tcb
  ac_tcb* ptcb;
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
 * Create a thread with a stack, and the initial psr
 *
 * If stack_size is <= 0 a "default" stack size will be used.
 *
 * If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 * otherwise an error is returned and the thread is not created.
 *
 * Return 0 on success !0 if an error.
 */
static ac_tcb* thread_create(ac_size_t stack_size, ac_u32 psr,
    void*(*entry)(void*), void* entry_arg) {
  ac_tcb* ptcb = AC_NULL;
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
  ptcb->sp = init_stack_frame(pstack + stack_size, psr, entry_trampoline, ptcb);

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
static ac_tcb* next_tcb(ac_tcb* pcur_tcb) {
  // Next thread
  ac_tcb* pnext = pcur_tcb->pnext_tcb;

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
ac_u8* thread_scheduler(ac_u8* sp) {
  // Save the current thread stack pointer
  pready->sp = sp;

  // Get the next tcb
  pready = next_tcb(pready);
  if (pready == &idle_tcb) {
    // Skip idle once, if idle is the only thread
    // then we'll execute this time.
    pready = next_tcb(pready);
  }

  return pready->sp;
}

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) {
  // Setup the stack frame exected by ac_reschedule
  // and branch to ac_reschedule
  __asm__ (
    "srsfd sp!,#31         // Save LR_irq and SPSR_irq to System mode stack\n"
    "cps   #31             // Switch to System mode\n"
    "push  {R0-R3,R12}     // Store other AAPCS registers\n"
    "and   r1, sp, #4      // R1 will be 0 if already on 64 bit boundry\n"
    "                      // or 4 if its not.\n"
    "sub   sp, sp, R1      // Substract 0 or 4 to align on 64 bit boundry\n"
    "push  {r1, lr}        // Push R1 to save realign facotr and LR to\n"
    "                      // keep it on 64 bit boundry\n"
    "// Reschedule the CPU\n"
    "ldr   r0, =ac_reschedule\n"
    "bx    r0\n"
  );
}

/**
 * Reschedule the CPU to the next ready thread. CAREFUL, this is
 * invoked by ac_init/srcs/ac_exception_irq_wrapper.S as such the
 * stack frame pointed to by sp must be identical.
 *
 * Interrupts will be disabled as the scheduler will be invoked
 */
void ac_reschedule(void) {
  // Start another thread
  __asm__ (
    "cpsid  i              // Disable IRQ\n"
    "mov   r0, sp          // Pass stackframe to thread_scheduler\n"
    "bl    thread_scheduler\n"
    "mov   sp, r0          // Replace sp with the return value\n"
    "pop   {r1,lr}         // Get realign factor back to R1 and pop LR\n"
    "add   sp, sp, r1      // Realign sp to old value\n"
    "pop   {r0-r3, r12}    // Restore registers\n"
    "rfefd sp!             // Return using RFE from System mode stack\n"
  );
}


/**
 * Early initialization of this module
 */
void ac_thread_early_init() {
  // Initialize the idle_stack
  ac_u32 idle_psr = PSR_MODE_SYS | PSR_FIQ_DISABLED | PSR_IRQ_ENABLED;
  init_stack_frame(idle_stack + sizeof(idle_stack), idle_psr, idle, AC_NULL);

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

  // Start a periodic timer for scheduling
  ac_start_periodic_timer(1000);
}

/**
 * Initialize this module
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  // Create array of the threads
  ac_u32 size = sizeof(ac_threads) + (max_threads * sizeof(ac_tcb));
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
 * Create a thread and invoke the entry_point passing
 * the parameter entry_arg.
 *
 * @param stack_size is <= 0 a "default" stack size will be used.
 *        If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 *        otherwise an error is returned and the thread is not created.
 *
 * @return an opaque value which is ZERO if an error.
 */
ac_thread_t ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg) {
  return (ac_thread_t)thread_create(stack_size,
      PSR_MODE_SYS | PSR_FIQ_DISABLED | PSR_IRQ_ENABLED, entry, entry_arg);
}
