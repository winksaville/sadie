/*
 * copyright 2016 wink saville
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

#include <ac_assert.h>
#include <ac_memmgr.h>

#define NDEBUG
#include <ac_debug_printf.h>

/**
 * Thread Control Block for x86
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter ??
 * Figure ??
 */
struct tcb_x86 {
  ac_u8 xsave_area[1024];
  ac_uptr stack_size;
  ac_u64* stack;
  ac_uptr thread_id;
  void*(*entry)(void*);
  void* entry_arg;
} __attribute__((__packed__, __aligned__(64)));

typedef struct tcb_x86 tcb_x86;

_Static_assert(sizeof(tcb_x86) >= 1024,
    L"x86 is not 1024 bytes");

typedef struct {
  ac_u32 max_count;
  tcb_x86 tcbs[];
} threads_x86;

#define AC_THREAD_ID_EMPTY (ac_uptr)-1
#define AC_THREAD_ID_NOT_EMPTY (ac_uptr)-2

static threads_x86* threads;

#if 0
static void* entry_trampoline(void* param) {
  // Invoke the entry point
  tcb_x86* ptcb = (tcb_x86*)param;
  ptcb->entry(ptcb->entry_arg);

  // Mark AC_THREAD_ID_EMPTY
  ac_uptr* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
  return AC_NULL;
}
#endif

/**
 * Initialize module
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  ac_uptr size = sizeof(threads_x86) + (max_threads * sizeof(tcb_x86));
  threads = ac_malloc(size);
  ac_assert(threads != AC_NULL);

  threads->max_count = max_threads;
  for (ac_u32 i = 0; i < threads->max_count; i++) {
    threads->tcbs[i].thread_id = AC_THREAD_ID_EMPTY;
  }
}

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) {
  // TODO: implementation
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

  // TODO: Create stack

  // Find an empty slot
  for (ac_u32 i = 0; i < threads->max_count; i++) {
    ac_uptr empty = AC_THREAD_ID_EMPTY;

    tcb_x86* ptcb = &threads->tcbs[i];
    ac_uptr* pthread_id = &ptcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_NOT_EMPTY, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      ptcb->entry = entry;
      ptcb->entry_arg = entry_arg;
      // TODO: CREATE THE THREAD
      // error |= pthread_create((pthread_t *)pthread_id, &attr, entry_trampoline, ptcb);
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

  return (ac_u32)error;
}
