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
 * All of the threads
 */
static ac_threads* pthreads;

/**
 * The idle task
 */
static ac_tcb* pidle_tcb;

/**
 * A NON-EMPTY circular linked list of tcbs to run,
 * it always contains at least the idle tcb
 * if nohting else.
 */
static ac_tcb* pready;

/**
 * The list of threads which are waiting for some
 * type of event??
 */
static ac_tcb* pwaiting_list;

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

#define PSR_IRQ_ENABLED  0x80
#define PSR_IRQ_DISABLED 0x80
#define PSR_FIQ_ENABLED  0x40
#define PSR_FIQ_DISABLED 0x40

/**
 * This structure matches the stack layout as expected
 * by ac_exception_irq_wrapper.S.
 *
 * Because ac_malloc always aligns on a 8 byte boundry
 * there is a padding work and the align_factor is 4.
 */
#pragma pack(push,1)
typedef struct {
  ac_u32 pc;                // Program Counter
  ac_u32 psr;               // Program Status word
  ac_u32 r0;                // R0
  ac_u32 r1;                // R1
  ac_u32 r2;                // R2
  ac_u32 r3;                // R3
  ac_u32 r12;               // R12
  ac_u32 padding;           // Padding to align stack on 8 byte boundry
  ac_u32 align_factor;      // A 4 as stack needed to be aligned
  ac_u32 lr;                // Extra align
} irq_wrapper_stack;
#pragma pack(pop)

ac_u8* init_stack_frame(ac_u8* tos, ac_u32 psr, void* (*entry)(void*),
    void* entry_arg) {
  irq_wrapper_stack* sp = (irq_wrapper_stack*)(tos - sizeof(irq_wrapper_stack));

  ac_static_assert(sizeof(void*) == sizeof(ac_u32),
      "Assumption that void* is 32 bits if false");
  sp->pc = (ac_u32)entry;
  sp->psr = psr;
  sp->r0 = (ac_u32)entry_arg;
  sp->r1 = 0;
  sp->r2 = 0;
  sp->r3 = 0;
  sp->r12 = 0;
  sp->padding = 0;
  sp->align_factor = 4;
  sp->lr = (ac_u32)entry;

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
  return AC_NULL;
}

/**
 * Remove any zombie threads recoverying the stack
 * and the tcb.
 */
static void remove_zombies(void) {
  ac_tcb* pzombie = AC_NULL;
  ac_tcb* ptail = pready;
  ac_tcb* phead = pready->pnext_tcb;
  while (phead != pidle_tcb) {
    ac_interrupts_cpu_disable();
    if (phead->thread_id == AC_THREAD_ID_ZOMBIE) {
      pzombie = phead;
      phead = phead->pnext_tcb;
      ptail->pnext_tcb = phead;
    } else {
      phead = phead->pnext_tcb;
    }
    ac_interrupts_cpu_enable();
    if (pzombie != AC_NULL) {
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
 * Add a tcb to pready
 */
static void add_pready(ac_tcb* ptcb) {
  ac_interrupts_cpu_disable();
  ptcb->pnext_tcb = pready->pnext_tcb;
  pready->pnext_tcb = ptcb;
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
  }
  return AC_NULL;
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
    ac_u32 empty = AC_THREAD_ID_EMPTY;

    ptcb = &pthreads->tcbs[i];
    ac_u32* pthread_id = &ptcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_STARTING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      ptcb->entry = entry;
      ptcb->entry_arg = entry_arg;
      ptcb->pstack = AC_NULL;
      __atomic_store_n(pthread_id, i, __ATOMIC_RELEASE);
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

  // Full Decending Stack, i.e. sp is pre-decremented when pushing and
  // post-decremented when popping.
  ac_u8* tos = pstack + stack_size;

  // Get the tcb and initialize the stack frame
  ptcb = get_tcb(entry, entry_arg);
  if (ptcb == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_TO_MANY_THREADS
    goto done;
  }
  ptcb->pstack = pstack;
  ptcb->sp = init_stack_frame(tos, PSR_MODE_SVC | PSR_FIQ_DISABLED
      | PSR_IRQ_ENABLED, entry_trampoline, ptcb);

  // Add this to pready
  add_pready(ptcb);

done:
  if (error != 0) {
    if (pstack != AC_NULL) {
      ac_free(pstack);
    }
  }

  return ptcb;
}

/**
 * Initialize module this is called before interrupts are enabled.
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  // Create two extra threads one for main and one for idle
  max_threads += 2;
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

  // Get the main tcb, no stack is needed because its
  // the one currently being used.
  ac_tcb* pmain_tcb = get_tcb(AC_NULL, AC_NULL);
  ac_assert(pmain_tcb != AC_NULL);
  ac_assert(pmain_tcb->pstack == AC_NULL);
  ac_assert(pmain_tcb->sp == AC_NULL);

  // Empty waiting list
  pwaiting_list = AC_NULL;

  // The current thread will be main
  pmain_tcb->pnext_tcb = pmain_tcb;
  pready = pmain_tcb;

  // Allocate the idle thread. It must never exit and
  // will always be on the ready list.
  pidle_tcb = thread_create(IDLE_STACK_SIZE, PSR_MODE_SVC | PSR_FIQ_DISABLED
      | PSR_IRQ_ENABLED, &idle, AC_NULL);

}

/**
 * Thread scheduler, for internal only, ASSUMES interrupts are DISABLED!
 *
 * @param pcur_sp is the stack of the current thread
 * @return the stack of the next thread to run
 */
ac_u8* ac_thread_scheduler(ac_u8* sp) {
  // Save the current
  pready->sp = sp;
  pready = pready->pnext_tcb;
  ac_u32* pthread_id = &pready->thread_id;
  ac_u32 thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  while(pready == pidle_tcb || thread_id == AC_THREAD_ID_ZOMBIE) {
    pready = pready->pnext_tcb;
    pthread_id = &pready->thread_id;
    thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  }
  return pready->sp;
}

/**
 * Create a thread and invoke the entry_point passing
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
  ac_tcb* ptcb = thread_create(stack_size,
      PSR_MODE_SVC | PSR_FIQ_DISABLED | PSR_IRQ_ENABLED, entry, entry_arg);
  return ptcb == AC_NULL ? 1 : 0;
}
