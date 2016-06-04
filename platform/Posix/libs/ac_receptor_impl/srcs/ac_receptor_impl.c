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

#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_printf.h>
#include <ac_thread.h>

#include <ac_debug_printf.h>

#include <semaphore.h>

#define RECEPTOR_STATE_UNUSED         0
#define RECEPTOR_STATE_INITIALIZING   1
#define RECEPTOR_STATE_ACTIVE         2
#define RECEPTOR_STATE_DEINITIALIZING 3

/**
 * Receptor structure
 */
typedef struct AcReceptor {
  sem_t semaphore;      // Posix semaphore
  ac_uint state;        // Current state
} AcReceptor;

typedef struct {
  ac_u32 max_count;
  AcReceptor receptors[];
} PosixAcReceptors;

PosixAcReceptors* receptor_array;

/**
 * Get a receptor and set its state to signaled
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
      sem_init(&receptor_array->receptors[i].semaphore, 0, 0);
      __atomic_store_n(pstate, RECEPTOR_STATE_ACTIVE, __ATOMIC_RELEASE);
      return preceptor;
    }
  }

  ac_printf("AcReceptor_create:-No receptors available\n");
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
    sem_destroy(&receptor->semaphore);
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
  // RACE with create/destroy, user beware.
  return (ac_u32)sem_wait(&receptor->semaphore);
}

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 */
void AcReceptor_signal(AcReceptor* receptor) {
  // RACE with create/destroy, user beware.
  sem_post(&receptor->semaphore);
}

/**
 * Signal the receptor and if there is another thread waiting
 * yield the CPU os it might run.
 *
 * @param receptor to signal
 */
void AcReceptor_signal_yield_if_waiting(AcReceptor* receptor) {
  sem_post(&receptor->semaphore);
  ac_thread_yield();
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
  ac_assert(max_receptors > 0);

  ac_u32 size = sizeof(PosixAcReceptors) + (max_receptors * sizeof(AcReceptor));
  receptor_array = ac_malloc(size);
  ac_assert(receptor_array != AC_NULL);

  receptor_array->max_count = max_receptors;
  for (ac_u32 i = 0; i < receptor_array->max_count; i++) {
    //sem_init(&receptor_array->receptors[i].semaphore, 0, 0);
    ac_uint* pstate = &receptor_array->receptors[i].state;
    __atomic_store_n(pstate, RECEPTOR_STATE_UNUSED, __ATOMIC_RELEASE);
  }
}
