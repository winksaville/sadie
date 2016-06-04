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

#define NDEBUG

/**
 * CAUTION: There is a RACE between entities which create/destroy receptors
 * and users that wait/signal. I'm using state to handle racing between
 * creators/destroyers so that should be ok. But threads that are signaling
 * are could definitely have reference to "stale" receptors. The worst problem
 * is that the receptor could be destroyed and recreated and they would think
 * they were talking to the old entity and instead are talking with the new
 * one. The same would happen for waiting, but in the typical case the entity
 * that waits is also the one that creates and destorys so in the nonimal case
 * that wont' be a problem. But if the waiter is separate from the creator
 * and destroyer the same problem exists.
 *
 * On solution might we reference counters or maybe instance id, will have to see.
 */
#include <ac_receptor.h>

#include <interrupts_x86.h>
#include <thread_x86.h>

#include <ac_assert.h>
#include <ac_debug_assert.h>
#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_thread.h>

#define RECEPTOR_STATE_UNUSED         0
#define RECEPTOR_STATE_INITIALIZING   1
#define RECEPTOR_STATE_ACTIVE         2
#define RECEPTOR_STATE_DEINITIALIZING 3

#define RECEPTOR_NO_ONE_WAITING ((ac_thread_hdl_t)0)
#define RECEPTOR_SIGNALED       ((ac_thread_hdl_t)1)


/**
 * Receptor structure
 */
typedef struct AcReceptor {
  ac_thread_hdl_t thdl; // Thread handle waiting
  ac_uint state;        // Current state
} AcReceptor;

typedef struct {
  ac_u32 max_count;
  AcReceptor receptors[];
} X86AcReceptor;

X86AcReceptor* receptor_array;

/**
 * Get a receptor and set its state to NOT signaled
 *
 * @return AC_NULL if unable to allocate a receptor
 */
AcReceptor* AcReceptor_get(void) {
  // Find an empty slot
  for (ac_uint i = 0; i < receptor_array->max_count; i++) {
    AcReceptor* preceptor = &receptor_array->receptors[i];
    ac_uint* pstate = &preceptor->state;
    ac_uint expected = RECEPTOR_STATE_UNUSED;
    if (__atomic_compare_exchange_n(pstate, &expected,
        RECEPTOR_STATE_INITIALIZING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {

      receptor_array->receptors[i].thdl = RECEPTOR_NO_ONE_WAITING;

      __atomic_store_n(pstate, RECEPTOR_STATE_ACTIVE, __ATOMIC_RELEASE);
      return preceptor;
    }
  }

  ac_debug_printf("AcReceptor_create:-No receptors available\n");
  return AC_NULL;
}

/**
 * Return a receptor
 */
void AcReceptor_ret(AcReceptor* receptor) {
  ac_uint* pstate = &receptor->state;
  while (__atomic_load_n(pstate, __ATOMIC_ACQUIRE) == RECEPTOR_STATE_INITIALIZING) {
    ac_thread_yield();
  }
  ac_uint expected = RECEPTOR_STATE_ACTIVE;
  if (__atomic_compare_exchange_n(pstate, &expected,
        RECEPTOR_STATE_DEINITIALIZING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {

    receptor->thdl = RECEPTOR_NO_ONE_WAITING;

    __atomic_store_n(pstate, RECEPTOR_STATE_UNUSED, __ATOMIC_RELEASE);
  }
}

/**
 * Wait for the receptor to be signaled only one entity can wait
 * on a receptor at a time. If the receptor has already been signaled
 * AcReceptor_wait will return immediately.
 *
 * @return 0 if successfully waited, !0 indicates an error such as
 * another thread was already waiting. This only happens if a program
 * error and there is more than one entity trying to wait.
 */
ac_u32 AcReceptor_wait(AcReceptor* receptor) {
  ac_thread_hdl_t my_thdl = ac_thread_get_cur_hdl();
  ac_thread_hdl_t* preceptor_thdl = &receptor->thdl;

  /*
   * Three possibilities:
   *  Case 1: It's not signaled and no one is waiting, so wait until signaled
   *  Case 2: It's already been signaled in which case it marked no one waiting and we continue
   *  Case 3: Someone else is waiting on it, this is currently an error
   */
  ac_thread_hdl_t expected = RECEPTOR_NO_ONE_WAITING;
  if (__atomic_compare_exchange_n(preceptor_thdl, &expected,
      my_thdl, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
    // Case 1: not signed and no one is waiting, so wait
    ac_debug_printf("AcReceptor_wait: waiting my_thdl=0x%x\n", ac_thread_get_cur_hdl());
    thread_make_not_ready(my_thdl);
    ac_debug_printf("AcReceptor_wait: resuming my_thdl=0x%x\n", ac_thread_get_cur_hdl());
    // We've been awoken, indicate no one is waiting
    __atomic_store_n(preceptor_thdl, RECEPTOR_NO_ONE_WAITING, __ATOMIC_RELEASE);
    //restore_intr(flags);
    return 0;
  } else {
    // Expecting it to be already signaled
    expected = RECEPTOR_SIGNALED;
    if (__atomic_compare_exchange_n(preceptor_thdl, &expected,
        RECEPTOR_NO_ONE_WAITING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
      // Case 2: Its signaled, so marked no one waiting and continue
      ac_debug_printf("AcReceptor_wait: continuing my_thdl=0x%x\n", my_thdl);
      return 0;
    } else {
      // Case 3: Someone else is waiting on it, this is currently an error
      ac_debug_printf("AcReceptor_wait: BUG someone else is waiting my_thdl=0x%x maybe=0x%x\n",
          my_thdl, __atomic_load_n(preceptor_thdl, __ATOMIC_ACQUIRE));
      intr(3);
      // Bummer, soneone is already waiting, probably a programmer error
      // TODO: This is not guaranteed to fail, but good enough for now.
      // TODO: Add ac_debug_fail("xxxx");
      ac_assert(*preceptor_thdl == RECEPTOR_NO_ONE_WAITING ||
          *preceptor_thdl == RECEPTOR_SIGNALED);
      return 1;
    }
  }
}

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 */
void AcReceptor_signal(AcReceptor* receptor) {
  ac_thread_hdl_t* preceptor_thdl = &receptor->thdl;

#if 0

  // No loop but interrupts disabled

  ac_uint flags = disable_intr();
  {
    ac_thread_hdl_t thdl = __atomic_load_n(preceptor_thdl, __ATOMIC_ACQUIRE);
    if (thdl == RECEPTOR_NO_ONE_WAITING) {
      __atomic_store_n(preceptor_thdl, RECEPTOR_SIGNALED, __ATOMIC_RELEASE);
    } else if (thdl == RECEPTOR_SIGNALED) {
      // Already signaled
    } else {
      // Sone one is waiting
      thread_make_ready(thdl);
    }
  }
  restore_intr(flags);

#else

  // Interrupts are not disabled but a loop

  ac_thread_hdl_t expected = RECEPTOR_NO_ONE_WAITING;
  while (!__atomic_compare_exchange_n(preceptor_thdl, &expected,
      RECEPTOR_SIGNALED, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
    // Could be a racing with another thread, we assume thread_make_ready
    // will do the right thing.
    ac_thread_hdl_t thdl = __atomic_load_n(preceptor_thdl, __ATOMIC_ACQUIRE);
    if (thdl == RECEPTOR_NO_ONE_WAITING) {
      ac_debug_printf("AcReceptor_signal: No one is waiting, try again\n");
      continue;
    } else if (thdl == RECEPTOR_SIGNALED) {
      ac_debug_printf("AcReceptor_signal:-Already signaled,\n");
      break;
    } else {
      ac_debug_printf("AcReceptor_signal: make ready by thdl=0x%x yield to thdl=0x%x\n",
          ac_thread_get_cur_hdl(), thdl);
      thread_make_ready(thdl);
      break;
    }
  }

#endif

  // Receptor is now signaled
  ac_debug_printf("AcReceptor_signal: signaled by thdl=0x%x\n", ac_thread_get_cur_hdl());
  return;
}

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 */
void AcReceptor_signal_yield_if_waiting(AcReceptor* receptor) {
  ac_thread_hdl_t* preceptor_thdl = &receptor->thdl;

#if 0

  // No loop but interrupts disabled

  ac_uint flags = disable_intr();
  {
    ac_thread_hdl_t thdl = __atomic_load_n(preceptor_thdl, __ATOMIC_ACQUIRE);
    if (thdl == RECEPTOR_NO_ONE_WAITING) {
      __atomic_store_n(preceptor_thdl, RECEPTOR_SIGNALED, __ATOMIC_RELEASE);
    } else if (thdl == RECEPTOR_SIGNALED) {
      // Already signaled
    } else {
      // Sone one is waiting
      ac_uint rslt = thread_make_ready(thdl);
      if (rslt == 0) {
          ac_debug_printf("AcReceptor_signal_yiw:-made ready by thdl=0x%x yield to thdl=0x%x\n",
          //    ac_thread_get_cur_hdl(), thdl);
          ac_thread_yield();
      }
    }
  }
  restore_intr(flags);

#else

  // Interrupts are not disabled but a loop

  ac_thread_hdl_t expected = RECEPTOR_NO_ONE_WAITING;
  while (!__atomic_compare_exchange_n(preceptor_thdl, &expected,
      RECEPTOR_SIGNALED, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
    // Could be a racing with another thread, we assume thread_make_ready
    // will do the right thing.
    ac_thread_hdl_t thdl = __atomic_load_n(preceptor_thdl, __ATOMIC_ACQUIRE);
    if (thdl == RECEPTOR_NO_ONE_WAITING) {
      ac_debug_printf("AcReceptor_signal_yiw: No one is waiting, try again\n");
      continue;
    } else if (thdl == RECEPTOR_SIGNALED) {
      ac_debug_printf("AcReceptor_signal_yiw:-Already signaled,\n");
      break;
    } else {
      ac_debug_printf("AcReceptor_signal_yiw: make ready by thdl=0x%x yield to thdl=0x%x\n",
          ac_thread_get_cur_hdl(), thdl);
      ac_uint rslt = thread_make_ready(thdl);
      if (rslt == 0) {
          ac_debug_printf("AcReceptor_signal_yiw:-made ready by thdl=0x%x yield to thdl=0x%x\n",
              ac_thread_get_cur_hdl(), thdl);
          ac_thread_yield();
      }
      break;
    }
  }

#endif

  // Receptor is now signaled
  ac_debug_printf("AcReceptor_signal_yiw: signaled by thdl=0x%x\n", ac_thread_get_cur_hdl());
  return;
}


/**
 * Initialize this module early, must be
 * called before receptor_init
 */
__attribute__((__constructor__))
void AcReceptor_early_init(void) {
}

/**
 * Initialize this module
 */
void AcReceptor_init(ac_u32 max_receptors) {
  ac_debug_printf("AcReceptor_init:+max=%d\n", max_receptors);

  ac_assert(max_receptors > 0);

  ac_u32 size = sizeof(X86AcReceptor) + (max_receptors * sizeof(AcReceptor));
  receptor_array = ac_malloc(size);
  ac_assert(receptor_array != AC_NULL);

  receptor_array->max_count = max_receptors;
  for (ac_u32 i = 0; i < receptor_array->max_count; i++) {
    ac_uint* pstate = &receptor_array->receptors[i].state;
    __atomic_store_n(pstate, RECEPTOR_STATE_UNUSED, __ATOMIC_RELEASE);
  }

  ac_debug_printf("AcReceptor_init:-\n");
}
