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

typedef struct ac_tcb {
  struct ac_tcb*  thread_id;
  void*(*entry)(void*);
  void* entry_arg;
  ac_u8   *pstack;
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

  // Marke AC_THREAD_ID_EMPTY
  ac_tcb** pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
  return AC_NULL;
}

/**
 * Initialize module
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  ac_u32 size = sizeof(ac_threads) + (max_threads * sizeof(ac_tcb));
  pthreads = ac_malloc(size);
  ac_assert(pthreads != AC_NULL);

  pthreads->max_count = max_threads;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    pthreads->tcbs[i].thread_id = (ac_tcb*)AC_THREAD_ID_EMPTY;
    pthreads->tcbs[i].pstack = AC_NULL;
  }
}

/**
 * Get an execution thread and invoke the entry_point passing
 * the parameter entry_arg.
 *
 * If stack_size is <= 0 a "default" stack size will be used.
 *
 * If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 * otherwise an error is returned and the thread is not created.
 *
 * Return 0 on success !0 if an error.
 */
ac_u32 ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg) {
  ac_u8* pstack = AC_NULL;
  int error = 0;

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
  pstack += stack_size;

  // Find an empty slot
  error = 1; // Error return if no empty slots
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    ac_uptr empty = AC_THREAD_ID_EMPTY;

    ac_tcb* ptcb = &pthreads->tcbs[i];
    ac_tcb** pthread_id = &ptcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_NOT_EMPTY, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      ptcb->entry = entry;
      ptcb->entry_arg = entry_arg;
      ptcb->pstack = pstack;
      __atomic_store_n(pthread_id, ptcb, __ATOMIC_RELEASE);
      error = 0;
      break;
    }
  }


done:
  if (error != 0) {
    if (pstack != AC_NULL) {
      ac_free(pstack);
    }
  }

  return (ac_u32)error;
}
