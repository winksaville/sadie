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
  pthread_t thread_id;
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
  pthread_t* pthread_id = &ptcb->thread_id;
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
  ac_tcb* pthe_tcb = AC_NULL;
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

    ac_tcb* pcur_tcb = &pthreads->tcbs[i];
    pthread_t* pthread_id = &pcur_tcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_NOT_EMPTY, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      pcur_tcb->entry = entry;
      pcur_tcb->entry_arg = entry_arg;
      error |= pthread_create((pthread_t *)pthread_id, &attr,
          entry_trampoline, pcur_tcb);
      ac_assert(*pthread_id != AC_THREAD_ID_EMPTY);
      ac_assert(*pthread_id != AC_THREAD_ID_NOT_EMPTY);
      if (error == 0) {
        pthe_tcb = pcur_tcb;
        break;
      } else {
        // Mark as empty and try again, although probably won't work
        __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
      }
    }
  }

  pthread_attr_destroy(&attr);

done:
  rslt.hdl = (ac_thread_hdl_t)pthe_tcb;
  rslt.status = (rslt.hdl != 0) ? 0 : 1;
  return (ac_thread_rslt_t)rslt;
}
