/*
 * Copyright 2016 Wink Saville
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define NDEBUG

#include <ac_mem_pool/tests/incs/test.h>

#include <ac_assert.h>
#include <ac_comp_mgr.h>
#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_memset.h>
#include <ac_mpscfifo.h>
#include <ac_mem.h>
#include <ac_mem_pool.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>
#include <ac_tsc.h>

#define MEMS_PER_THREAD 20
#define MEMS_TSC_COUNT  MEMS_PER_THREAD

#if AC_PLATFORM == pc_x86_64
extern ac_uint remove_zombies(void);
extern void print_ready_list(const char* str);
extern ac_u32 get_ready_length(void);
#endif

ac_u32 ready_length(void) {
#if AC_PLATFORM == pc_x86_64
  // This only returns a value if SUPPORT_READY_LENGTH is defined
  // when compliing thread_x86.c. Undefined by default and returns 0.
  return get_ready_length();
#else
  return 0;
#endif
}

typedef struct ClientParams {
  ac_bool running;
  ac_bool done;
  ac_u8* mem;
  ac_u64 error_count;
  ac_u64 count;
  AcReceptor* receptor_ready;
  AcReceptor* receptor_waiting;
  AcReceptor* receptor_work_complete;
  AcReceptor* receptor_done;
} ClientParams;

void* client(void* param) {
  ac_debug_printf("client:+param=%p\n", param);
  ClientParams* cp = (ClientParams*)param;

  ac_assert((cp->receptor_waiting = AcReceptor_get()) != AC_NULL);

  // Signal we're ready
  AcReceptor_signal(cp->receptor_ready);

  // While we're not done wait for a signal to do work
  // do the work and signal work is complete.
  ac_bool* ptr_done = &cp->done;
  while (__atomic_load_n(ptr_done, __ATOMIC_ACQUIRE) == AC_FALSE) {
    AcReceptor_wait(cp->receptor_waiting);

    if (!cp->done) {
      cp->count += 1;
      ac_u8** ptr_mem = &cp->mem;
      ac_u8* m = __atomic_exchange_n(ptr_mem, AC_NULL, __ATOMIC_ACQUIRE);
      if (m == AC_NULL) {
        cp->error_count += 1;
      } else {
        AcMemPool_ret_mem(m);
        AcReceptor_signal(cp->receptor_work_complete);
      }
    }
  }

  AcReceptor_ret(cp->receptor_waiting);

  // Signal we're done
  AcReceptor_signal(cp->receptor_done);

  ac_debug_printf("client:-param=%p\n", param);
  return AC_NULL;
}

/**
 * Test mem pools being having the calling thread create
 * subbe the axle
 * and passed the number of spokes
 *
 * return AC_TRUE if an error.
 */
ac_bool test_mem_pool_multiple_threads(ac_u32 thread_count) {
  ac_bool error = AC_FALSE;
  ClientParams* client_params = AC_NULL;
  ac_u64 count_sum = 0;
  ac_u64 count = 0;
  ac_u64 loops = 0;
  ac_debug_printf("test_mem_pool_multiple_threads:+ thread_count=%d\n", thread_count);

#if AC_PLATFORM == VersatilePB
  //ac_printf("test_mem_pool_multiple_threads: VersatilePB threading not working, skipping\n");
  ac_printf("test_mem_pool_multiple_threads: skipping\n");
#else

  AcReceptor* work_complete = AcReceptor_get();
  ac_assert(work_complete != AC_NULL);

  AcMemPoolCountSize mpcs[1];
  mpcs[0].count = thread_count; // + 10; //* 10;
  mpcs[0].data_size = 1;

  AcMemPool* pool;
  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mpcs), mpcs, &pool) == AC_STATUS_OK);
  if (error) {
    goto done;
  }

  // Start clients
  client_params = ac_calloc(thread_count, sizeof(ClientParams));
  for (ac_u32 t = 0; t < thread_count; t++) {
    ClientParams* cp = &client_params[t];

    cp->done = AC_FALSE;
    cp->mem = AC_NULL;
    cp->count = 0;
    cp->error_count = 0;
    ac_assert((cp->receptor_ready = AcReceptor_get()) != AC_NULL);
    ac_assert((cp->receptor_done = AcReceptor_get()) != AC_NULL);
    cp->receptor_work_complete = work_complete;

    ac_thread_rslt_t rslt = ac_thread_create(0, client, (void*)cp);
    error |= AC_TEST(rslt.status == AC_STATUS_OK);
    if (error) {
      cp->running = AC_FALSE;
    } else {
      cp->running = AC_TRUE;
      AcReceptor_wait(cp->receptor_ready);
      ac_debug_printf("test_mem_pool_multiple_threads: started cp=%p\n", cp);
    }
  }
  if (error) {
    ac_printf("test_mem_pool_multiple_threads: error starting, skip tests");
    goto done;
  }

  // For a few seconds have to clients do work on the memory
  //#define RUNTIME 10
  //ac_u64 end_tsc = ac_tscrd() + AcTime_nanos_to_ticks(AC_SEC_IN_NS * RUNTIME);
  //while (ac_tscrd() < end_tsc) {
  for (ac_u32 i = 0; i < 200000; i++) {
    for (ac_u32 t = 0; t < thread_count; t++) {
      loops += 1;

      ClientParams* cp = &client_params[t];
      ac_u8** ptr_cp_mem = &cp->mem;
      if (__atomic_load_n(ptr_cp_mem, __ATOMIC_ACQUIRE) == AC_NULL) {
        // Ask for some memory
        ac_u8* mem;
        AcMemPool_get_mem(pool, 1, (void**)&mem);
        if (mem != AC_NULL) {
          // Got it, pass it to client
          __atomic_store_n(ptr_cp_mem, mem, __ATOMIC_RELEASE);

          // Signal client receptor
          AcReceptor_signal(cp->receptor_waiting);
          count += 1;
        } else {
          //ac_printf("no mem t=%d\n", t);
          //AcReceptor_wait(work_complete);
          ac_thread_yield();
        }
      }
    }

#if 0
    // Waiting for work to complete causes the pc_x86_64
    // scheduler to be unfair when there are >= thread_count
    // memory buffers created when thread_count is > 1. For
    // Posix it doesn't matter.
    //
    // What happens is that we end up in the ready list in
    // reverse order and the first thread, t = 0, never
    // processes any memory buffers. Threads 1 to thread_count - 2
    // processes a single memory buffer and the last thread,
    // thread_count -1 processes all of the rest!!

    AcReceptor_wait(work_complete);
#else
    // If we yield then its fair as every thread will get to
    // processes its memory buffers.

    ac_thread_yield();
#endif
  }

done:
  // Cleanup
  ac_debug_printf("test_mem_pool_multiple_threads: Done cleanup, loops=%ld\n", loops);
  for (ac_u32 t = 0; t < thread_count; t++) {
    ClientParams* cp = &client_params[t];

    if (cp->running) {
      // Tell client its done working and wait until its done
      __atomic_store_n(&cp->done, AC_TRUE, __ATOMIC_RELEASE);
      AcReceptor_signal(cp->receptor_waiting);
      AcReceptor_wait(cp->receptor_done);

      ac_debug_printf("test_mem_pool_multiple_threads: cleanup cp=%p cp->error_count=%ld cp->count=%ld\n",
          cp, cp->error_count, cp->count);
      count_sum += cp->count;

      // Collect error signal
      error |= (cp->error_count != 0);
    }

    // Cleanup
    AcReceptor_ret(cp->receptor_ready);
    AcReceptor_ret(cp->receptor_done);
  }
  AcReceptor_ret(work_complete);
  ac_free(client_params);

  // Let idle run to clean up zombies with Platform == pc_x86_64
  // TODO: Shouldn't have to do this
  ac_thread_wait_ns(100000);
#endif

  ac_debug_printf("test_mem_pool_multiple_threads:- error=%d rl=%d count=%ld count_sum=%ld loops=%ld\n\n",
      error, ready_length(), count, count_sum, loops);
  return error;
}
