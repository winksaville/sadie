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

#define NDEBUG
#include <ac_debug_printf.h>

#include <pthread.h>
#include <unistd.h>
#include <sched.h>

typedef struct {
  ac_uptr thread_id;
  void*(*entry)(void*);
  void* entry_arg;
} ac_tcb;

typedef struct {
  ac_u32 max_count;
  ac_tcb tcbs[];
} ac_threads;

#define AC_THREAD_ID_EMPTY (ac_uptr)-1
#define AC_THREAD_ID_NOT_EMPTY (ac_uptr)-2

static ac_threads* pthreads;

static void* entry_trampoline(void* param) {
  // Invoke the entry point
  ac_tcb* ptcb = (ac_tcb*)param;
  ptcb->entry(ptcb->entry_arg);

  // Mark AC_THREAD_ID_EMPTY
  ac_uptr* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
  return AC_NULL;
}

/**
 * Initialize module
 */
void ac_thread_init(ac_u32 max_threads) {
  // Verify that pthread_t is <= sizeof(ac_uptr)
  ac_static_assert(sizeof(pthread_t) <= sizeof(ac_uptr),
      "Expect pthread_t to be the size of a pointer");

  ac_assert(max_threads > 0);

  ac_u32 size = sizeof(ac_threads) + (max_threads * sizeof(ac_tcb));
  pthreads = ac_malloc(size);
  ac_assert(pthreads != AC_NULL);

  pthreads->max_count = max_threads;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    pthreads->tcbs[i].thread_id = AC_THREAD_ID_EMPTY;
  }
}

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) {
  sched_yield();
}


/**
 * Get an execution thread and invoke the entry_point passing
 * the parameter entry_arg.
 *
 * If stack_size is 0 a "default" stack size will be used.
 *
 * Return 0 on success !0 if an error.
 */
ac_u32 ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg) {
  int error = 0;
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  if (stack_size > 0) {
    error |= pthread_attr_setstacksize(&attr, (size_t)stack_size);
    if (error != 0) {
      goto done;
    }
  }

  // Find an empty slot
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    ac_uptr empty = AC_THREAD_ID_EMPTY;

    ac_tcb* ptcb = &pthreads->tcbs[i];
    ac_uptr* pthread_id = &ptcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_NOT_EMPTY, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      ptcb->entry = entry;
      ptcb->entry_arg = entry_arg;
      error |= pthread_create((pthread_t *)pthread_id, &attr,
          entry_trampoline, ptcb);
      ac_assert(*pthread_id != AC_THREAD_ID_EMPTY);
      ac_assert(*pthread_id != AC_THREAD_ID_NOT_EMPTY);
      if (error == 0) {
        break;
      } else {
        // Mark as empty and try again, although probably won't work
        __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
      }
    }
  }

  pthread_attr_destroy(&attr);

done:
  return (ac_u32)error;
}
