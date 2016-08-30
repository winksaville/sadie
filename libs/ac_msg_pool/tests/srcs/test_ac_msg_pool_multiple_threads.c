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

#define NDEBUG

#include <ac_msg_pool/tests/incs/test.h>

#include <ac_comp_mgr.h>
#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_msg_pool.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_thread.h>
#include <ac_tsc.h>

#define MSGS_PER_THREAD 20
#define MSGS_TSC_COUNT  MSGS_PER_THREAD

#if AC_PLATFORM == pc_x86_64
extern AcUint remove_zombies(void);
extern void print_ready_list(const char* str);
extern AcU32 get_ready_length(void);
#endif

AcU32 ready_length(void) {
#if AC_PLATFORM == pc_x86_64
  // This only returns a value if SUPPORT_READY_LENGTH is defined
  // when compliing thread_x86.c. Undefined by default and returns 0.
  return get_ready_length();
#else
  return 0;
#endif
}

struct MsgTscData {
  AcU64 waiting_count;
  AcU64 sent_tsc;
};

struct MsgTsc {
  AcU32 waiting_count;
  AcU32 ready_length;
  AcU64 sent_tsc;
  AcU64 recv_tsc;
  AcU64 done_tsc;
};

typedef struct {
  AcComp comp;
  AcU8 name[16];
  AcCompMgr* cm;
  AcCompInfo* ci;
  AcBool stop_processing_msgs;
  AcU64 count;
  struct MsgTsc msg_tsc[MSGS_TSC_COUNT];
} mptt_params;

AcBool mptt_process_msg(AcComp* this, AcMsg* msg) {
  if (msg->op.operation != AC_INIT_CMD && msg->op.operation != AC_DEINIT_CMD) {
    AcU64 recv_tsc = ac_tscrd();
    mptt_params* params = (mptt_params*)this;

    AcUint idx = params->count++ % AC_ARRAY_COUNT(params->msg_tsc);
    struct MsgTscData* mtd = (struct MsgTscData*)msg->extra;
    struct MsgTsc* mt = &params->msg_tsc[idx];
    mt->waiting_count = mtd->waiting_count;
    mt->ready_length = ready_length();
    mt->sent_tsc = mtd->sent_tsc;
    mt->recv_tsc = recv_tsc;

    ac_debug_printf("mptt_process_msg:- msg->arg1=%ld msg->arg2=%ld count=%ld\n",
        msg->arg1, msg->arg2, params->count);

    // Return message
    AcMsgPool_ret_msg(msg);

    mt->done_tsc = ac_tscrd();

    __atomic_thread_fence(__ATOMIC_RELEASE);
  }
  return AC_TRUE;
}

/**
 * Count the message in the pool
 */
AcU32 count_msgs(AcMsgPool* mp, AcU32 msg_count) {
  AcU32 count = 0;

  ac_debug_printf("count_msgs:+ msg_count=%d\n", msg_count);

  if (msg_count > 0) {
    AcMsg** array = ac_malloc(sizeof(AcMsg*) * msg_count);

    AcMsg* msg;
    AcU32 idx = 0;
    while ((msg = AcMsgPool_get_msg(mp)) != AC_NULL) {
      array[idx] = msg;
      count += 1;
      idx = count % msg_count;
    }
    for (AcU32 idx = 0; idx < count && idx < msg_count; idx++) {
      AcMsgPool_ret_msg(array[idx]);
    }

    ac_free(array);
  }

  ac_debug_printf("count_msgs:- count=%d\n", count);
  return count;
}

/**
 * Test msg pools being used by multiple threads
 *
 * return AC_TRUE if an error.
 */
AcBool test_msg_pool_multiple_threads(AcU32 thread_count, AcU32 comps_per_thread) {
  AcBool error = AC_FALSE;

  ac_printf("test_msg_pool_multiple_threads:+ thread_count=%d comps_per_thread=%d rl=%d\n",
      thread_count, comps_per_thread, ready_length());
#if AC_PLATFORM == VersatilePB
  ac_printf("test_msg_pool_multiple_threads: VersatilePB threading not working, skipping\n");
#else
  AcStatus status;
  AcMsgPool mp;
  AcU32 total_comp_count = thread_count * comps_per_thread;

  // Create a msg pool. The msg_count must be at
  // at least twice the number of total components
  // because each component has a fifo and the first
  // message returned by the queue will be a stub
  // created when the fifo was created. Hence we loose
  // the use of one message until the fifo is deinited.
  //
  // Since there is one queue per thread in this
  // test we create a message pool with twice the
  // number threads as the msg_count
  AcU32 msg_count = total_comp_count * 2;
  status = AcMsgPool_init(&mp, msg_count, sizeof(struct MsgTscData));
  error |= AC_TEST(status == AC_STATUS_OK);
  error |= AC_TEST(count_msgs(&mp, msg_count) == msg_count);
  ac_debug_printf("test_msg_pool_multiple_threads:+total_comp_count=%d msg_count=%d\n",
      total_comp_count, msg_count);

  mptt_params** params = ac_malloc(sizeof(mptt_params) * thread_count);

  // Create the component manager with appropriate
  // number of theads and comps_per_thread
  AcCompMgr cm;
  status = AcCompMgr_init(&cm, thread_count, comps_per_thread, 0);
  error |= AC_TEST(status == AC_STATUS_OK);

  // Add the components to the component manager
  for (AcU32 i = 0; i < total_comp_count; i++) {
    ac_debug_printf("test_msg_pool_multiple_threads: init %d\n", i);
    params[i] = ac_malloc(sizeof(mptt_params));
    error |= AC_TEST(params[i] != AC_NULL);
    if (error) {
      return error;
    }

    params[i]->cm = &cm;
    ac_sprintf(params[i]->name, sizeof(params[i]->name), "mptt%d_process_msg", i);
    params[i]->comp.name = params[i]->name;
    params[i]->comp.process_msg = mptt_process_msg;
    params[i]->ci = AcCompMgr_add_comp(&cm, &params[i]->comp);
    error |= AC_TEST(params[i]->ci != AC_NULL);
  }

  // Send the threads messages which they will return to our pool
  //
  // Note: See [this](https://github.com/winksaville/sadie/wiki/test-ac_msg_pool-on-Posix)
  // and other pages in the wiki for additional information.
  AcU64 start = ac_tscrd();
  ac_debug_printf("test_msg_pool_multiple_threads: sending msgs\n");
  const AcU32 max_waiting_count = 100000;
  AcU32 waiting_count = 0;
  for (AcU32 msg = 0; !error && (msg < MSGS_PER_THREAD); msg++) {
    ac_debug_printf("test_msg_pool_multiple_threads: waiting_count=%ld msg=%d\n",
        waiting_count, msg);
    for (AcU32 i = 0; !error && (i < thread_count); i++) {
        ac_debug_printf("test_msg_pool_multiple_threads: %d waiting_count=%ld TOP Loop\n",
            i, waiting_count);
      AcMsg* msg = AcMsgPool_get_msg(&mp);
      while (!error && (msg == AC_NULL)) {
        if (waiting_count++ >= max_waiting_count) {
          ac_printf("test_msg_pool_multiple_threads: %d waiting_count=%ld ERROR, no msgs avail\n",
              i, waiting_count);
          // Report the failure.
          error |= AC_TEST(waiting_count < max_waiting_count);
          break;
        }
        ac_thread_yield();
        msg = AcMsgPool_get_msg(&mp);
      }
      if (msg != AC_NULL) {
        struct MsgTscData* mtd = (struct MsgTscData*)msg->extra;
        mtd->waiting_count = waiting_count;
        mtd->sent_tsc = ac_tscrd();
        AcCompMgr_send_msg(params[i]->ci, msg);
      }
    }
  }

  // Deinit the component manager which will stop and wait until
  // they are done.
  AcCompMgr_deinit(&cm);

  AcU64 stop = ac_tscrd();
  ac_printf("test_msg_pool_multiple_threads: done in %.9t\n", stop - start);

  // If we cleaned up the threads then all of the messages should be back
  error |= AC_TEST(count_msgs(&mp, msg_count) == msg_count);

  ac_debug_printf("test_msg_pool_multiple_threads: waiting_count=%ld\n",
      waiting_count);

  // Display MsgTsc's.
  for (AcU32 thrd = 0; thrd < thread_count; thrd++) {
    mptt_params* cur = params[thrd];
    AcUint max = AC_ARRAY_COUNT(cur->msg_tsc);
    AcUint idx;
    AcUint count;
    if (cur->count < max) {
      idx = 0;
      count = cur->count;
    } else {
      count = max;
      idx = cur->count % max;
    }
    ac_printf("params[%d]=%p\n", thrd, cur);
    for (; count-- > 0; idx = (idx + 1) % max) {
      struct MsgTsc* msg_tsc = &cur->msg_tsc[idx];

      ac_printf("%d: sent=%ld wc=%d rl=%d travel=%.9t ret=%.9t\n",
          idx, msg_tsc->sent_tsc, msg_tsc->waiting_count, msg_tsc->ready_length,
          msg_tsc->recv_tsc - msg_tsc->sent_tsc,
          msg_tsc->done_tsc - msg_tsc->recv_tsc);
    }
  }

  // Cleanup
  for (AcU32 thrd = 0; thrd < thread_count; thrd++) {
    ac_free(params[thrd]);
  }

  ac_free(params);

#endif

  ac_printf("test_msg_pool_multiple_threads:- error=%d rl=%d\n", error, ready_length());
  return error;
}
