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
#include <ac_thread.h>
#include <ac_tsc.h>

#define MSGS_PER_THREAD 20
#define MSGS_TSC_COUNT  MSGS_PER_THREAD

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

#if 0
struct MsgTsc {
  ac_u32 waiting_count;
  ac_u32 ready_length;
  ac_u64 sent_tsc;
  ac_u64 recv_tsc;
  ac_u64 done_tsc;
  ac_bool extra_error;
  ac_u32 extra_error_index;
  ac_u8 extra_error_value;
  ac_u8 extra_error_expected_value;
};

typedef struct {
  AcComp comp;
  AcCompMgr* cm;
  AcCompInfo* ci;
  ac_bool stop_processing_mems;
  ac_u32 extra_size;
  ac_u8  extra_value;
  ac_u64 count;
  struct MsgTsc mem_tsc[MSGS_TSC_COUNT];
} mptt_params;

static ac_bool mptt_process_mem(AcComp* this, AcMsg* msg) {
  ac_u64 recv_tsc = ac_tscrd();
  mptt_params* params = (mptt_params*)this;

  ac_uint idx = params->count++ % AC_ARRAY_COUNT(params->mem_tsc);
  struct MsgTsc* mt = &params->mem_tsc[idx];
  mt->waiting_count = mem->arg1;
  mt->ready_length = ready_length();
  mt->sent_tsc = mem->arg2;
  mt->recv_tsc = recv_tsc;
  mt->extra_error = AC_FALSE;

  if (params->extra_size != 0) {
    for (ac_u32 i = 0; i < params->extra_size; i++) {
      mt->extra_error |= mem->data[i] != params->extra_value;
      if (mt->extra_error) {
        mt->extra_error_index = i;
        mt->extra_error_value = mem->extra_data[i];
        mt->extra_error_expected_value = params->extra_value;
        ac_debug_printf("mptt_process_mem: err mem->arg1=%ld mem->arg2=%ld count=%ld "
            " extra_error=%b extra_error_value=0x%x extra_error_expected_value=0x%x\n",
            mem->arg1, mem->arg2, params->count,
            mt->extra_error, mt->extra_error_value, mt->extra_error_expected_value);
        break;
      }
    }
  }
  ac_debug_printf("mptt_process_mem:- mem->arg1=%ld mem->arg2=%ld count=%ld\n",
      mem->arg1, mem->arg2, params->count);

  // Return message
  AcMsg_ret(mem);

  mt->done_tsc = ac_tscrd();

  __atomic_thread_fence(__ATOMIC_RELEASE);
  return AC_TRUE;
}

/**
 * Count the message in the pool
 */
ac_u32 count_mems(AcMsgPool* mp, ac_u32 mem_count) {
  ac_u32 count = 0;

  ac_debug_printf("count_mems:+ mem_count=%d\n", mem_count);

  if (mem_count > 0) {
    AcMsg** array = ac_malloc(sizeof(AcMsg*) * mem_count);

    AcMsg* mem;
    ac_u32 idx = 0;
    while ((mem = AcMsg_get(mp)) != AC_NULL) {
      array[idx] = mem;
      count += 1;
      idx = count % mem_count;
    }
    for (ac_u32 idx = 0; idx < count && idx < mem_count; idx++) {
      AcMsg_ret(array[idx]);
    }

    ac_free(array);
  }

  ac_debug_printf("count_mems:- count=%d\n", count);
  return count;
}
#endif

/**
 * Test mem pools being used by multiple threads
 *
 * return AC_TRUE if an error.
 */
ac_bool test_mem_pool_multiple_threads(ac_u32 thread_count, ac_u32 comps_per_thread,
    ac_u32 extra_size) {
  ac_bool error = AC_FALSE;
  ac_printf("test_mem_pool_multiple_threads:+ thread_count=%d comps_per_thread=%d extra_size=%d rl=%d\n",
      thread_count, comps_per_thread, extra_size, ready_length());
#if 10 //AC_PLATFORM == VersatilePB
  //ac_printf("test_mem_pool_multiple_threads: VersatilePB threading not working, skipping\n");
  ac_printf("test_mem_pool_multiple_threads: skipping\n");
#else

  ac_u32 total_comp_count = thread_count * comps_per_thread;

  // Create a mem pool. The mem_count must be at
  // at least twice the number of total components
  // because each component has a fifo and the first
  // message returned by the queue will be a stub
  // created when the fifo was created. Hence we loose
  // the use of one message until the fifo is deinited.
  //
  // Since there is one queue per thread in this
  // test we create a message pool with twice the
  // number threads as the mem_count
  ac_u32 mem_count = total_comp_count * 2;
  AcMsgPool* mp = AcMsgPool_create_extra(mem_count, extra_size);
  error |= AC_TEST(mp != AC_NULL);
  error |= AC_TEST(count_mems(mp, mem_count) == mem_count);
  ac_debug_printf("test_mem_pool_multiple_threads:+total_comp_count=%d mem_count=%d\n",
      total_comp_count, mem_count);

  mptt_params** params = ac_malloc(sizeof(mptt_params) * thread_count);

  // Create the component manager with appropriate
  // number of theads and comps_per_thread
  AcCompMgr* cm = AcCompMgr_init(thread_count, comps_per_thread, 0);
  error |= AC_TEST(cm != AC_NULL);

  // Add the components to the component manager
  for (ac_u32 i = 0; i < total_comp_count; i++) {
    ac_debug_printf("test_mem_pool_multiple_threads: init %d\n", i);
    params[i] = ac_malloc(sizeof(mptt_params));
    error |= AC_TEST(params[i] != AC_NULL);
    if (error) {
      return error;
    }

    params[i]->cm = cm;
    params[i]->comp.process_mem = mptt_process_mem;
    params[i]->ci = AcCompMgr_add_comp(cm, &params[i]->comp, extra_size);
    params[i]->extra_size = extra_size;
    params[i]->extra_value = 0x5a;
    error |= AC_TEST(params[i]->ci != AC_NULL);
  }

  // Send the threads messages which they will return to our pool
  //
  // Note: See [this](https://github.com/winksaville/sadie/wiki/test-ac_mem_pool-on-Posix)
  // and other pages in the wiki for additional information.
  ac_u64 start = ac_tscrd();
  ac_debug_printf("test_mem_pool_multiple_threads: sending mems\n");
  const ac_u32 max_waiting_count = 100000;
  ac_u32 waiting_count = 0;
  for (ac_u32 message = 0; !error && (message < MSGS_PER_THREAD); message++) {
    ac_debug_printf("test_mem_pool_multiple_threads: waiting_count=%ld message=%d\n",
        waiting_count, message);
    for (ac_s32 i = 0; !error && (i < thread_count); i++) {
        ac_debug_printf("test_mem_pool_multiple_threads: %d waiting_count=%ld TOP Loop\n",
            i, waiting_count);
      AcMsg* mem = AcMsg_get(mp);
      while (!error && (mem == AC_NULL)) {
        if (waiting_count++ >= max_waiting_count) {
          ac_printf("test_mem_pool_multiple_threads: %d waiting_count=%ld ERROR, no mems avail\n",
              i, waiting_count);
          // Report the failure.
          error |= AC_TEST(waiting_count < max_waiting_count);
          break;
        }
        ac_thread_yield();
        mem = AcMsg_get(mp);
      }
      if (mem != AC_NULL) {
        mem->arg1 = waiting_count;
        mem->arg2 = ac_tscrd();
        if (extra_size != 0) {
          ac_memset(mem->extra_data, params[i]->extra_value, params[i]->extra_size);
        }
        AcCompMgr_send_mem(params[i]->cm, params[i]->ci, mem);
      }
    }
  }

  // Deinit the component manager which will stop and wait until
  // they are done.
  AcCompMgr_deinit(cm);

  ac_u64 stop = ac_tscrd();
  ac_printf("test_mem_pool_multiple_threads: done in %.9t\n", stop - start);

  // If we cleaned up the threads then all of the messages should be back
  error |= AC_TEST(count_mems(mp, mem_count) == mem_count);

  ac_debug_printf("test_mem_pool_multiple_threads: waiting_count=%ld\n",
      waiting_count);

  // Display MsgTsc's.
  for (ac_u32 thrd = 0; thrd < thread_count; thrd++) {
    mptt_params* cur = params[thrd];
    ac_uint max = AC_ARRAY_COUNT(cur->mem_tsc);
    ac_uint idx;
    ac_uint count;
    if (cur->count < max) {
      idx = 0;
      count = cur->count;
    } else {
      count = max;
      idx = cur->count % max;
    }
    ac_printf("params[%d]=%p\n", thrd, cur);
    for (; count-- > 0; idx = (idx + 1) % max) {
      struct MsgTsc* mem_tsc = &cur->mem_tsc[idx];

      error |= AC_TEST(mem_tsc->extra_error == AC_FALSE);

      ac_printf("%d: sent=%ld wc=%d rl=%d travel=%.9t ret=%.9t extra_error=%d\n",
          idx, mem_tsc->sent_tsc, mem_tsc->waiting_count, mem_tsc->ready_length,
          mem_tsc->recv_tsc - mem_tsc->sent_tsc,
          mem_tsc->done_tsc - mem_tsc->recv_tsc,
          mem_tsc->extra_error);
    }
  }

  // Cleanup
  for (ac_u32 thrd = 0; thrd < thread_count; thrd++) {
    ac_free(params[thrd]);
  }

  ac_free(params);

#endif

  ac_printf("test_mem_pool_multiple_threads:- error=%d rl=%d\n", error, ready_length());
  return error;
}
