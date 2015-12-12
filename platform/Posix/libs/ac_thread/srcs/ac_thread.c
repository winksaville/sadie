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

#include <pthread.h>


typedef struct {
  ac_u32 max_count;
  ac_uptr thread_id[];
} ac_threads;

#define AC_THREAD_ID_EMPTY (ac_uptr)-1
#define AC_THREAD_ID_NOT_EMPTY (ac_uptr)-2

static ac_threads* pthreads;

/**
 * Initialize module
 */
void ac_thread_init(ac_u32 max_threads) {
  // Verify that pthread_t is <= sizeof(ac_uptr)
  ac_static_assert(sizeof(pthread_t) <= sizeof(ac_uptr),
      "Expect pthread_t to be the size of a pointer");

  ac_assert(max_threads > 0);

  ac_u32 size = sizeof(ac_threads) + (max_threads * sizeof(ac_uptr));
  pthreads = ac_malloc(size);
  ac_assert(pthreads != AC_NULL);

  pthreads->max_count = max_threads;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    pthreads->thread_id[i] = AC_THREAD_ID_EMPTY;
  }
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

    ac_uptr* pthread_id = &pthreads->thread_id[i];
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_NOT_EMPTY, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      error |= pthread_create((pthread_t *)pthread_id, &attr,
          entry, entry_arg);
      ac_assert(*pthread_id != AC_THREAD_ID_EMPTY);
      ac_assert(*pthread_id != AC_THREAD_ID_NOT_EMPTY);
      if (error != 0) {
        __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
      }
    }
  }

done:
  if (error != 0) {
    pthread_attr_destroy(&attr);
  }

  return (ac_u32)error;
}
