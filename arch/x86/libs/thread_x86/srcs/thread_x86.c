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
#include <thread_x86.h>
#include <thread_x86_debug_ctx_switch.h>

#include <apic_x86.h>
#include <cpuid_x86.h>
#include <interrupts_x86.h>
#include <interrupts_x86_print.h>
#include <native_x86.h>
#include <reset_x86.h>

#include <ac_assert.h>
#include <ac_bits.h>
#include <ac_debug_assert.h>
#include <ac_intmath.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_putchar.h>
#include <ac_tsc.h>

#include <ac_printf.h>

#define NDEBUG
#include <ac_debug_printf.h>

#define STATIC static

#define RESCHEDULE_ISR_INTR       0xfe
#define TIMER_RESCHEDULE_ISR_INTR 0xfd

// 1 billionth of a second is a nanosecond
#define NANOSECS 1000000000ll

// Default slice in nanoseconds
#define SLICE_DEFAULT_NANOSECS    10000000ll // 10ms

// The default number of ticks for a scheduling slice
STATIC ac_u64 slice_default;

STATIC ac_u64 timer_reschedule_isr_counter;

typedef struct tcb_x86 {
  ac_s32 thread_id;
  struct tcb_x86* pnext_tcb;
  struct tcb_x86* pprev_tcb;
  void*(*entry)(void*);
  void* entry_arg;
  ac_u8* pstack;
  ac_u8* sp;
  ac_u16 ss;
  ac_u64 slice;
  ac_u64 slice_deadline;
} tcb_x86;

typedef struct threads {
  struct threads* pnext;
  struct threads* pprev;
  ac_u32 max_count;
  tcb_x86 tcbs[];
} ac_threads;

#define DEFAULT_FLAGS 0x00200246

#define AC_THREAD_ID_EMPTY (ac_u32)-1
#define AC_THREAD_ID_STARTING (ac_u32)-2
#define AC_THREAD_ID_ZOMBIE (ac_u32)-3

/**
 * The idle thread
 */
STATIC ac_u8 idle_stack[AC_THREAD_STACK_MIN] __attribute__ ((aligned(64)));
STATIC tcb_x86* pidle_tcb;

/**
 * The main thread
 */
STATIC tcb_x86* pmain_tcb;

/**
 * A NON-EMPTY circular linked list of tcbs to run,
 * it always contains at least the idle tcb, if nohting else.
 */
STATIC tcb_x86* pready;

/**
 * All of the threads
 */
STATIC ac_u32 total_threads;
STATIC ac_threads* pthreads;

#define NUM_SAVED_REGS 15
struct saved_regs {
  ac_u64 rax;
  ac_u64 rdx;
  ac_u64 rcx;
  ac_u64 rbx;
  ac_u64 rsi;
  ac_u64 rdi;
  ac_u64 r8;
  ac_u64 r9;
  ac_u64 r10;
  ac_u64 r11;
  ac_u64 r12;
  ac_u64 r13;
  ac_u64 r14;
  ac_u64 r15;
  ac_u64 rbp;
};

struct full_stack_frame {
  union {
    struct saved_regs regs;
    ac_u64 regs_array[NUM_SAVED_REGS];
  };
  struct intr_frame iret_frame;
} __attribute__ ((__packed__));

#define FULL_STACK_FRAME_SIZE 20 * sizeof(ac_u64)
ac_static_assert(sizeof(struct full_stack_frame) == FULL_STACK_FRAME_SIZE,
    "inital_stack_frame is not " AC_XSTR(FULL_STACK_FRAME_SIZE) " bytes in size");

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
/**
 * Print full stack frame
 */
void print_full_stack_frame(char* str, struct full_stack_frame* fsf) {
  if (str != AC_NULL) {
    ac_printf("%s:", str);
  }
  ac_printf("fsf=0x%p\n", fsf);
  ac_printf(" rax: 0x%lx 0x%p\n", fsf->regs.rax, &fsf->regs.rax);
  ac_printf(" rdx: 0x%lx 0x%p\n", fsf->regs.rdx, &fsf->regs.rdx);
  ac_printf(" rcx: 0x%lx 0x%p\n", fsf->regs.rcx, &fsf->regs.rcx);
  ac_printf(" rbx: 0x%lx 0x%p\n", fsf->regs.rbx, &fsf->regs.rbx);
  ac_printf(" rsi: 0x%lx 0x%p\n", fsf->regs.rsi, &fsf->regs.rsi);
  ac_printf(" rdi: 0x%lx 0x%p\n", fsf->regs.rdi, &fsf->regs.rdi);
  ac_printf("  r8: 0x%lx 0x%p\n", fsf->regs.r8,  &fsf->regs.r8);
  ac_printf("  r9: 0x%lx 0x%p\n", fsf->regs.r9,  &fsf->regs.r9);
  ac_printf(" r10: 0x%lx 0x%p\n", fsf->regs.r10, &fsf->regs.r10);
  ac_printf(" r11: 0x%lx 0x%p\n", fsf->regs.r11, &fsf->regs.r11);
  ac_printf(" r12: 0x%lx 0x%p\n", fsf->regs.r12, &fsf->regs.r12);
  ac_printf(" r13: 0x%lx 0x%p\n", fsf->regs.r13, &fsf->regs.r13);
  ac_printf(" r14: 0x%lx 0x%p\n", fsf->regs.r14, &fsf->regs.r14);
  ac_printf(" r15: 0x%lx 0x%p\n", fsf->regs.r15, &fsf->regs.r15);
  ac_printf(" rbp: 0x%lx 0x%p\n", fsf->regs.rbp, &fsf->regs.rbp);
  print_intr_frame(AC_NULL, &fsf->iret_frame);
}
#endif

void print_tcb_list(const char* str, tcb_x86* phead) {
  ac_uint flags = disable_intr();
  {
    if (str != AC_NULL) {
      ac_printf(str);
    }
    tcb_x86* pcur = phead;
    do {
      ac_printf("%x:%d ", pcur, pcur->thread_id);
      pcur = pcur->pnext_tcb;
    } while (pcur != phead);
    ac_printf("\n");
  }
  restore_intr(flags);
}

void print_ready_list(const char* str) {
  print_tcb_list(str, pready);
}

/**
 * Initialize tcb it with the entry and entry_arg.
 *
 * @param entry routine to call to start thread
 * @param entry_arg is the argument to pass to entry
 *
 * Return AC_NULL if an error, i.e. nono available
 */
STATIC void tcb_init(tcb_x86* ptcb, ac_s32 thread_id,
    void*(*entry)(void*), void* entry_arg) {
  ptcb->entry = entry;
  ptcb->entry_arg = entry_arg;
  ptcb->pnext_tcb = AC_NULL;
  ptcb->pprev_tcb = AC_NULL;
  ptcb->slice = slice_default;
  ptcb->pstack = AC_NULL;
  ptcb->sp = AC_NULL;
  ac_s32* pthread_id = &ptcb->thread_id;
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
STATIC tcb_x86* get_tcb(void*(*entry)(void*), void* entry_arg) {
  tcb_x86* ptcb;

  // There must always be at least one ac_threads
  ac_assert(pthreads != AC_NULL);

  // Search all of the ac_threads for an empty tcb
  ac_threads* pcur = pthreads;
  do {
    // Find an empty tcb;
    for (ac_u32 i = 0; i < pcur->max_count; i++) {
      ptcb = &pcur->tcbs[i];

      ac_u32 empty = AC_THREAD_ID_EMPTY;
      ac_s32* pthread_id = &ptcb->thread_id;
      ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
          AC_THREAD_ID_STARTING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
      if (ok) {
        // Found an empty tcb, initialize and return it
        tcb_init(ptcb, i, entry, entry_arg);
        return ptcb;
      }
    }
    pcur = pcur->pnext;
  } while (pcur != pthreads);

  // No empty tcbs
  return AC_NULL;
}

/**
 * Next tcb
 */
STATIC __inline__ tcb_x86* next_tcb(tcb_x86* pcur_tcb) {
  // Next thread
  tcb_x86* pnext = pcur_tcb->pnext_tcb;

  // Skip any ZOMBIE threads it is ASSUMED the list
  // has at least one non ZOMBIE thread, the idle thread,
  // so this is guarranteed not to be an endless loop.
  ac_s32* pthread_id = &pnext->thread_id;
  ac_s32 thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  while(thread_id == AC_THREAD_ID_ZOMBIE) {
    // Skip the ZOMBIE
    pnext = pnext->pnext_tcb;
    pthread_id = &pnext->thread_id;
    thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  }

  return pnext;
}


/**
 * Add an initialized tcb before pcur
 *
 * @param pnew is a tcb which will before pcur.
 * @param pcur is a tcb in the list which will succeed pnew
 *
 * @return 0 if successful
 */
STATIC ac_uint add_tcb_before(tcb_x86* pnew, tcb_x86* pcur) {
  ac_uint rslt;
  ac_uint flags = disable_intr();

  tcb_x86* ptmp = pcur->pprev_tcb;
  if (pnew->pprev_tcb == AC_NULL) {
    pnew->pprev_tcb = ptmp;
    pnew->pnext_tcb = pcur;
    ptmp->pnext_tcb = pnew;
    pcur->pprev_tcb = pnew;
    //ac_printf("add_tcb_before: ret rslt=0, pcur=0x%x pnew=0x%x\n", pcur, pnew);
    rslt = 0;
  } else {
    //ac_printf("add_tcb_before: ret rslt=1, pcur=0x%x pnew=0x%x\n", pcur, pnew);
    rslt = 1;
  }

  restore_intr(flags);
  return rslt;
}

/**
 * Add an initialized tcb following pcur
 *
 * @param pnew is a tcb which will after pcur.
 * @param pcur is a tcb in the list which will preceed pnew
 *
 * @return 0 if successful
 */
STATIC ac_uint add_tcb_after(tcb_x86* pnew, tcb_x86* pcur) {
  ac_uint rslt;
  ac_uint flags = disable_intr();

  tcb_x86* ptmp = pcur->pnext_tcb;
  if (pnew->pnext_tcb == AC_NULL) {
    pnew->pnext_tcb = ptmp;
    pnew->pprev_tcb = pcur;
    ptmp->pprev_tcb = pnew;
    pcur->pnext_tcb = pnew;
    //ac_printf("add_tcb_after: ret rslt=0, pcur=0x%x pnew=0x%x\n", pcur, pnew);
    rslt = 0;
  } else {
    //ac_printf("add_tcb_after: ret rslt=1, pcur=0x%x pnew=0x%x\n", pcur, pnew);
    rslt = 1;
  }

  restore_intr(flags);
  return rslt;
}

/**
 * Remove a tcb from the list, we assume the list will
 * NEVER be empty as idle will always be present.
 *
 * @param pcur is a tcb to be removed
 *
 * @return pnext if successful, else AC_NULL
 */
STATIC tcb_x86* remove_tcb_intr_disabled(tcb_x86* pcur) {
  tcb_x86* pnext_tcb = pcur->pnext_tcb;
  if (pnext_tcb != AC_NULL) {
    tcb_x86* pprev_tcb = pcur->pprev_tcb;
    //ac_printf("remove_tcb_intr_disabled: pcur=%x pnext_tcb=%x pprev_tcb=%x\n",
    //    pcur, pnext_tcb, pprev_tcb);
    pprev_tcb->pnext_tcb = pnext_tcb;
    pnext_tcb->pprev_tcb = pprev_tcb;

    pcur->pnext_tcb = AC_NULL;
    pcur->pprev_tcb = AC_NULL;
  }
  return pnext_tcb;
}

/**
 * Remove a tcb from the list, we assume the list will
 * NEVER be empty as idle will always be present.
 *
 * @param pcur is a tcb to be removed
 */
STATIC void remove_tcb_from_ready_intr_disabled(tcb_x86* pcur) {
  if (pcur == pready) {
    //ac_printf("remove_tcb_from_ready_intr_disabled:+ yield pcur:%x == pready:%x", pcur, pready);
    //print_tcb_list(" ", pready);
    thread_yield(AC_TRUE);
    //ac_printf("remove_tcb_from_ready_intr_disabled: after yield pcur:%x == pready:%x", pcur, pready);
    //print_tcb_list(" ", pready);
  } else {
    //ac_printf("remove_tcb_from_ready_intr_disabled:+ before remove pcur:%x != pready:%x", pcur, pready);
    //print_tcb_list(" ", pready);
    remove_tcb_intr_disabled(pcur);
    //ac_printf("remove_tcb_from_ready_intr_disabled:+ after remove pcur:%x != pready:%x", pcur, pready);
    //print_tcb_list(" ", pready);
  }
}

static tcb_x86* ptimer_waiting_tcb;
static ac_u64 timer_waiting_until_tsc;

/**
 * Let timer mdify the tcb to be scheduled.
 * Interrupts disabled!
 */
STATIC tcb_x86* timer_scheduler_intr_disabled(tcb_x86* next_tcb, ac_u64 now) {
  if (__atomic_load_n(&ptimer_waiting_tcb, __ATOMIC_ACQUIRE) != AC_NULL) {
    // There is a tcb with a timer waiting to expire
    ac_u64 until_tsc = __atomic_load_n(&timer_waiting_until_tsc, __ATOMIC_ACQUIRE);
    if (now >= until_tsc) {
      // Timed out so schedule it.
      add_tcb_before(ptimer_waiting_tcb, next_tcb);
      next_tcb = ptimer_waiting_tcb;
      next_tcb->slice_deadline = now + next_tcb->slice;
      __atomic_store_n(&ptimer_waiting_tcb, AC_NULL, __ATOMIC_RELEASE);

      // TODO: Find the next timer that is going to timeout?

    } else if ((next_tcb->slice_deadline) > until_tsc) {
      next_tcb->slice_deadline = until_tsc;
    } else {
      // timer_waiting_until_tsc is in the far future
    }
  } else {
    // There are no tcb waiting for a timer to expire.
  }

  return next_tcb;
}

/**
 * Add a new timer wait
 */
static void pready_timer_wait_until_tsc(ac_u64 tsc) {
  ac_uint flags = disable_intr();
  {
    __atomic_store_n(&timer_waiting_until_tsc, tsc, __ATOMIC_RELEASE);
    __atomic_store_n(&ptimer_waiting_tcb, pready, __ATOMIC_RELEASE);
    remove_tcb_from_ready_intr_disabled(pready);
  }
  restore_intr(flags);
}

/**
 * Thread scheduler for reschedule_isr.
 * For internal only use only and ASSUMES
 * interrupts are DISABLED!
 *
 * @param sp is the stack of the current thread
 * @param ss is the stack segment of the current thread
 *
 * @return the tcb of the next thread to run
 */
__attribute__((__noinline__))
tcb_x86* thread_scheduler_intr_disabled(ac_bool remove_pready, ac_u8* sp, ac_u16 ss) {
  // For a consistent notion of now get it once in the scheduler
  ac_u64 now = ac_tscrd();

  //ac_printf("thread_scheduler_intr_disabled:+remove_pready=%d, sp=%x ss=%x pready=%x flags=%x now=%ld\n",
  //    remove_pready, sp, ss, pready, ((struct full_stack_frame*)(sp))->iret_frame.flags,
  //    now);

  // Save the current thread stack pointer
  pready->sp = sp;
  pready->ss = ss;

  // We assume the pready list is never empty as idle_tcb is
  // always present, thus this is never AC_NULL even when
  // removing a tcb.
  if (remove_pready) {
    //ac_printf("thread_scheduler_intr_disabled: true remove_pready=%d\n", remove_pready);

    // Make sure we never remove pidle_tcb!!
    ac_assert(pready != pidle_tcb);

    // Remove the pready tcb
    pready = remove_tcb_intr_disabled(pready);
  } else {
    // Just get the next tcb
    pready = next_tcb(pready);
  }

  // Check if pready is pointing at idle_tcb
  if (pready == pidle_tcb) {
    // Yep, skip idle unless it's the only thread
    // then we'll it will become pready and we'll
    // execute it this time.
    //ac_printf("thread_scheduler_intr_disabled: skip idle\n");
    pready = next_tcb(pready);
  }

  // Set the threads slice deadline
  pready->slice_deadline = now + pready->slice;

  // Let timer see if it has something that should be scheduled,
  // Note, it may modify pready->slice_deadline.
  pready = timer_scheduler_intr_disabled(pready, now);

  // Set the a new tsc deadline for this thread
  set_apic_timer_tsc_deadline(pready->slice_deadline);

  ac_assert(pready != AC_NULL);

  //print_tcb_list("thread_scheduler_intr_disabled:-", pready);
  return pready;
}

/**
 * Thread scheduler for timer_reschedule_isr.
 * For internal only use only and ASSUMES
 * interrupts are DISABLED!
 *
 * @param sp is the stack of the current thread
 * @param ss is the stack segment of the current thread
 *
 * @return the tcb of the next thread to run
 */
tcb_x86* timer_thread_scheduler_intr_disabled(ac_u8* sp, ac_u16 ss) {
  tcb_x86* ptcb = thread_scheduler_intr_disabled(AC_FALSE, (ac_u8*)sp, ss);
  __atomic_add_fetch(&timer_reschedule_isr_counter, 1, __ATOMIC_RELEASE);
  send_apic_eoi();
  return ptcb;
}

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) {
  thread_yield(AC_FALSE);
}

/**
 * Get current thread handle
 *
 */
ac_thread_hdl_t ac_thread_get_cur_hdl(void) {
  return (ac_thread_hdl_t)pready;
}

/**
 * @return timer_reschedule_isr_counter.
 */
ac_u64 get_timer_reschedule_isr_counter(void) {
  return __atomic_load_n(&timer_reschedule_isr_counter, __ATOMIC_ACQUIRE);
}

/**
 * Set timer_reschedule_isr_counter to value
 *
 * @param value is stored in counter
 */
void set_timer_reschedule_isr_counter(ac_u64 value) {
  return __atomic_store_n(&timer_reschedule_isr_counter, value, __ATOMIC_RELEASE);
}

/**
 * Initialize timer_rescheduler interupt
 */
STATIC void init_timer() {
  union apic_timer_lvt_fields_u lvtu = { .fields = get_apic_timer_lvt() };
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;

  // Verify that TSC_DEADLINE is enabled
  //
  // See "Intel 64 and IA-32 Architectures Software Developer's Manual"
  // Volume 3 chapter 10 "Advanded Programmable Interrupt Controller (APIC)"
  // Section 10.5.4.1 "TSC-Deadline Mode"

  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  if (AC_GET_BITS(ac_u32, out_ecx, 24, 1) != 1) {
    ac_printf("CPU does not support TSC-Deadline mode\n");
    reset_x86();
  }

  lvtu.fields.vector = TIMER_RESCHEDULE_ISR_INTR; // interrupt vector
  lvtu.fields.disable = AC_FALSE; // interrupt enabled
  lvtu.fields.mode = 2;     // TSC-Deadline
  set_apic_timer_lvt(lvtu.fields);

  slice_default = AC_U64_DIV_ROUND_UP(SLICE_DEFAULT_NANOSECS * ac_tsc_freq(), NANOSECS);

  __atomic_store_n(&timer_reschedule_isr_counter, 0, __ATOMIC_RELEASE);
  set_apic_timer_tsc_deadline(ac_tscrd() + slice_default);

  // Init waiting globals
  __atomic_store_n(&ptimer_waiting_tcb, AC_NULL, __ATOMIC_RELEASE);
  __atomic_store_n(&timer_waiting_until_tsc, 0ll, __ATOMIC_RELEASE);

  ac_printf("init_timer:-slice_default_nanosecs=%ld slice_default=%ld\n",
      SLICE_DEFAULT_NANOSECS, slice_default);
}

/**
 * A routine that calls the actual thread entry
 *
 * @param param is a pointer to the tcb_x86.
 *
 * @return AC_NULL
 */
__attribute__((__noreturn__))
STATIC void* entry_trampoline(void* param) {
  // Invoke the entry point
  tcb_x86* ptcb = (tcb_x86*)param;
  ptcb->entry(ptcb->entry_arg);

  // Mark as zombie
  ac_s32* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_ZOMBIE, __ATOMIC_RELEASE);

  ac_thread_yield();

  // Never gets here because the code is ZOMBIE
  // But we need to prove it to the compiler
  while (AC_TRUE);
}

/**
 * Remove any zombie threads recoverying the stack
 * and the tcb.
 */
ac_uint remove_zombies(void) {
  tcb_x86* pzombie = AC_NULL;
  tcb_x86* pnext_tcb;
  ac_uint count = 0;

  ac_uptr flags = disable_intr();
  // Loop through the list of ready tcbs removing
  // ZOMBIE entries. We'll start at idle_tcb as
  // the tail since its guaranteed to be on the
  // list and not a ZOMBIE.
  tcb_x86* phead = pidle_tcb->pnext_tcb;
  while (phead != pidle_tcb) {
    //ac_uptr flags = disable_intr();

    ac_s32* pthread_id = &phead->thread_id;
    ac_s32 thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
    if (thread_id == AC_THREAD_ID_ZOMBIE) {
      // phead is a ZOMBIE, remove it from the
      // list advancing the head past it.
      pzombie = phead;
      pnext_tcb = remove_tcb_intr_disabled(pzombie);
    } else {
      pnext_tcb = phead->pnext_tcb;
    }
    // Advance head
    phead = pnext_tcb;

    //restore_intr(flags);

    if (pzombie != AC_NULL) {
      // Free the ZOMBIE's stack and mark it EMPTY
      if (pzombie->pstack != AC_NULL) {
        ac_free(pzombie->pstack);
      }
      __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
      count += 1;
      pzombie = AC_NULL;
    }
  }

  restore_intr(flags);
  return count;
}

/**
 * Idle routine, invoked if no other thread is ready.
 * It must never exit.
 */
STATIC void* idle(void* param) {
  while (AC_TRUE) {
    remove_zombies();
    // TODO: Goto a lower power state
  }
  return AC_NULL;
}


/**
 * Initialize the stack frame assuming no stack switching
 */
STATIC void init_stack_frame(ac_u8* pstack, ac_uptr stack_size, ac_uptr flags,
    void* (*entry)(void*), void* entry_arg, ac_u8** psp, ac_u16 *pss) {
  ac_u8* tos = pstack + stack_size;
  struct full_stack_frame* sf =
    (struct full_stack_frame*)(tos - sizeof(struct full_stack_frame));

  ac_static_assert(sizeof(void*) == sizeof(ac_uptr),
      "Assumption that void* is sizeof ac_uptr is false");

  // Be sure stack is on a 16 byte boundry
  ac_assert(((ac_uptr)pstack & 0xf) == 0);
  ac_assert(((ac_uptr)stack_size & 0xf) == 0);

  // fill the stack with its address for debugging
  //ac_uptr* pfill = (ac_uptr*)pstack;
  //for (ac_uint i = 0; i < stack_size / sizeof(ac_uptr); i++) {
  //  pfill[i] = (ac_uptr)&pfill[i];
  //}

  //print_full_stack_frame("thread_x86 init_stack_frame before init", sf);

  //for (ac_uint i = 0; i < AC_ARRAY_COUNT(sf->regs_array); i++) {
  //  sf->regs_array[i] = i;
  //}

  sf->regs.rdi = (ac_u64)entry_arg;
  sf->iret_frame.ip = (ac_uptr)entry;
  sf->iret_frame.cs = 0x8;
  sf->iret_frame.flags = flags;

  sf->iret_frame.sp = (ac_uptr)(tos);
  sf->iret_frame.ss = 0x10;

  // print_full_stack_frame("thread_x86 init_stack_frame after init", sf);

  *psp = (ac_u8*)sf;
  *pss = sf->iret_frame.ss;
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
//__attribute__((noinline))
STATIC tcb_x86* thread_create(ac_size_t stack_size, ac_uptr flags,
    void*(*entry)(void*), void* entry_arg) {
  ac_uint sv_flags = disable_intr();
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
  stack_size = (stack_size + 0xff) & ~0xff;

  pstack = ac_malloc(stack_size);
  if (pstack == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_OOM
    goto done;
  }
  ac_assert(((ac_uptr)pstack & 0xf) == 0);

  // Get the tcb and initialize the stack frame
  ptcb = get_tcb(entry, entry_arg);
  if (ptcb == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_TO_MANY_THREADS
    goto done;
  }
  ptcb->pstack = pstack;
  init_stack_frame(pstack, stack_size, flags, entry_trampoline, ptcb,
      &ptcb->sp, &ptcb->ss);

  // Add this after pready
  add_tcb_after(ptcb, pready);

done:
  if (error != 0) {
    if (pstack != AC_NULL) {
      ac_free(pstack);
    }
  }

#if AC_FALSE
  ac_printf("thread_create: pstack=0x%x stack_size=0x%x tos=0x%x\n",
      pstack, stack_size, pstack + stack_size);
  ac_printf("thread_create:-ptcb=0x%x ready: ", ptcb);
  print_tcb_list(AC_NULL, pready);
#endif

  restore_intr(sv_flags);
  return ptcb;
}

/**
 * Create a thread and invoke the entry passing entry_arg. If
 * the entry routine returns the thread is considered dead
 * and will not be rescheduled and its stack is reclamined.
 * Any other global memory associated with the thread still
 * exists and is left untouched.
 *
 * @param stack_size is 0 a "default" stack size will be used.
 * @param entry is the routine to run
 * @param entry_arg is the argument passed to entry.
 *
 * @return a ac_thread_rslt contains a status and an opaque ac_thread_hdl_t.
 *         if rslt.status == 0 the thread was created and ac_thread_hdl_t
 *         is valid.
 */
ac_thread_rslt_t ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg) {
  ac_thread_rslt_t rslt;

  rslt.hdl = (ac_thread_hdl_t)thread_create(stack_size, get_flags(), entry, entry_arg);
  rslt.status = (rslt.hdl != 0) ? 0 : 1;
  return rslt;
}

/**
 * Make the thread not ready,
 *
 * @param hdl is an opaque thread handle
 */
void thread_make_not_ready(ac_thread_hdl_t hdl) {
  ac_uint flags = disable_intr();
  {
    remove_tcb_from_ready_intr_disabled((tcb_x86*)hdl);
  }
  restore_intr(flags);
}

/**
 * Make the thread ready
 *
 * @param hdl is an opaque thread handle
 *
 * @return 0 if successful, 1 if it is already on a list
 */
ac_uint thread_make_ready(ac_thread_hdl_t hdl) {
  return add_tcb_after((tcb_x86*)hdl, pready);
}

/**
 * The current thread waits for some number of nanosecs.
 */
void ac_thread_wait_ns(ac_u64 nanosecs) {
  ac_u64 secs = nanosecs / NANOSECS;
  ac_u64 sub_secs = nanosecs % NANOSECS;
  ac_u64 ticks = secs * ac_tsc_freq();
  ticks += AC_U64_DIV_ROUND_UP(sub_secs * ac_tsc_freq(), NANOSECS);

  pready_timer_wait_until_tsc(ac_tscrd() + ticks);
}

/**
 * The current thread waits for some number of ticks.
 */
void ac_thread_wait_ticks(ac_u64 ticks) {
  pready_timer_wait_until_tsc(ac_tscrd() + ticks);
}

/**
 * Early initialization of this module
 */
void ac_thread_early_init() {
  // Initialize timer first, the main reason
  // is it set slice_default.
  init_timer();

  // Initialize reschedule isr
  set_intr_handler(RESCHEDULE_ISR_INTR, reschedule_isr);
  set_intr_handler(TIMER_RESCHEDULE_ISR_INTR, timer_reschedule_isr);

  // Allocate the initial array
  total_threads = 0;
  pthreads = AC_NULL;
  ac_thread_init(2);
  ac_assert(pthreads != AC_NULL);

  pidle_tcb = &pthreads->tcbs[0];
  pmain_tcb = &pthreads->tcbs[1];

  // Initialize idle and main tcbs
  tcb_init(pidle_tcb, 0, idle, AC_NULL);
  tcb_init(pmain_tcb, 1, AC_NULL, AC_NULL);

  // Initialize idle's stack
  init_stack_frame(idle_stack, sizeof(idle_stack), DEFAULT_FLAGS, idle, pidle_tcb,
      &pidle_tcb->sp, &pidle_tcb->ss);

  // Add main as the initial pready list
  pmain_tcb->pnext_tcb = pmain_tcb;
  pmain_tcb->pprev_tcb = pmain_tcb;
  pready = pmain_tcb;

  // Add idle to the pready list
  add_tcb_after(pidle_tcb, pready);

  ac_printf("ac_thread_early_init: pmain=0x%lx pidle=0x%lx\n", pmain_tcb, pidle_tcb);
  print_tcb_list("ac_thread_early_init:-ready: ", pready);
}

/**
 * Initialize this module
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  ac_uint flags = disable_intr();
  if (max_threads > total_threads) {
    // Create array of the threads
    ac_u32 count = max_threads - total_threads;
    ac_u32 size = sizeof(ac_threads) + (count * sizeof(tcb_x86));
    ac_threads* pnew = ac_malloc(size);
    ac_assert(pnew != AC_NULL);
    pnew->max_count = count;

    // Initialize new entries to AC_THREAD_EMPTY
    for (ac_u32 i = 0; i < count; i++) {
      tcb_init(&pnew->tcbs[i], AC_THREAD_ID_EMPTY, AC_NULL, AC_NULL);
    }

    if (pthreads == AC_NULL) {
      // Add frist set of threads
      pnew->pnext = pnew;
      pnew->pprev = pnew;
      pthreads = pnew;
    } else {
      // Add these new ones to the beginning of the list
      // by adding after the pthreads then move pthreads
      ac_threads* ptmp = pthreads->pnext;
      pnew->pnext = ptmp;
      pnew->pprev = pthreads;
      ptmp->pprev = pnew;
      pthreads->pnext = pnew;

      // Point pthreads at pnew to make it head of the list
      pthreads = pnew;
    }
    total_threads += max_threads;
  }
  restore_intr(flags);
}

