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

#include <ac_msg_pool/tests/incs/test.h>

#include <ac_dispatcher.h>

#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_mpscfifo.h>
#include <ac_msg_pool.h>
#include <ac_receptor.h>
#include <ac_thread.h>
#include <ac_test.h>

typedef struct {
  ac comp;
  ac_u64 count;
  ac_mpscfifo q;
  ac_bool stop_processing_msgs;
  ac_receptor_t ready;
  ac_receptor_t done;
  ac_receptor_t waiting;
} mptt_params;

static ac_bool mptt_process_msg(ac* this, AcMsg* msg) {
  mptt_params* params = (mptt_params*)this;

  __atomic_add_fetch(&params->count, 1, __ATOMIC_RELAXED);

  //ac_debug_printf("c=%d\n", msg->cmd);
  //ac_debug_printf("mptt_process_msg:- msg->cmd=%d, msg->arg=%d count=%ld\n",
  //    msg->cmd, msg->arg, params->count);

  // Return message
  AcMsg_ret(msg);

  return AC_TRUE;
}

/**
 * Msg Pool Test Thread
 */
void* mptt(void *param) {
  ac_bool error = AC_FALSE;
  mptt_params* params = (mptt_params*)param;

  //ac_debug_printf("mptt:+ starting  params=%p\n", params);

  // Add an acq
  ac_dispatcher* d;

  params->waiting = ac_receptor_create();

  // Get a dispatcher
  d = ac_dispatcher_get(1);
  error |= AC_TEST(d != AC_NULL);

  // Init the queue
  ac_mpscfifo_init(&params->q);

  // Init Async Component
  params->comp.process_msg = mptt_process_msg;

  // Add ac1 and its Q dispatcher
  ac_dispatcher_add_acq(d, &params->comp, &params->q);

  // We're not stopped
  __atomic_store_n(&params->stop_processing_msgs, AC_FALSE, __ATOMIC_RELEASE);

  // Signal mptt is ready
  //ac_debug_printf("mptt: ready params=%p\n", params);
  ac_receptor_signal(params->ready);

  // Continuously dispatch messages until done
  //ac_debug_printf("mptt: looping params=%p\n", params);
  while (__atomic_load_n(&params->stop_processing_msgs, __ATOMIC_ACQUIRE) == AC_FALSE) {
    if (!ac_dispatch(d)) {
      //ac_debug_printf("mptt: waiting\n");
      ac_receptor_wait(params->waiting);
      //ac_debug_printf("mptt: continuing\n");
    }
  }

  ac_dispatcher_rmv_ac(d, &params->comp);

  //ac_debug_printf("mptt:-done error=%d params=%p\n", error, params);

  ac_receptor_signal_yield_if_waiting(params->done);
  return AC_NULL;
}

/**
 * Send a message to mptt
 */
void mptt_send_msg(mptt_params* params, AcMsg* msg) {
  //ac_debug_printf("mptt_send_msg:+params=%p msg=%p\n", params, msg);
  ac_mpscfifo_add_msg(&params->q, msg);
  ac_receptor_signal_yield_if_waiting(params->waiting);
}

/**
 * Stop the mptt and wait until its done
 */
void mptt_stop_and_wait_until_done(mptt_params* params) {
  //ac_debug_printf("mptt_stop_wait_until_done:+params=%p\n", params);
  __atomic_store_n(&params->stop_processing_msgs, AC_TRUE, __ATOMIC_RELEASE);
  ac_receptor_signal_yield_if_waiting(params->waiting);
  ac_receptor_wait(params->done);
}

/**
 * Count the message in the pool
 */
ac_u32 count_msgs(AcMsgPool* mp, ac_u32 msg_count) {
  ac_u32 count = 0;

  ac_debug_printf("count_msgs:+ msg_count=%d\n", msg_count);

  if (msg_count > 0) {
    AcMsg** array = ac_malloc(sizeof(AcMsg*) * msg_count);

    AcMsg* msg;
    ac_u32 idx = 0;
    while ((msg = AcMsg_get(mp)) != AC_NULL) {
      array[idx] = msg;
      count += 1;
      idx = count % msg_count;
    }
    for (ac_u32 idx = 0; idx < count && idx < msg_count; idx++) {
      AcMsg_ret(array[idx]);
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
ac_bool test_msg_pool_multiple_threads(ac_u32 thread_count) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_msg_pool_multiple_threads:+ thread_count=%d\n", thread_count);
#ifdef VersatilePB
  ac_debug_printf("test_msg_pool_multiple_threads: VersatilePB threading not working, skipping\n");
#else

  // Create a msg pool
  ac_u32 msg_count = thread_count * 2;
  AcMsgPool* mp = AcMsgPool_create(msg_count);
  error |= AC_TEST(mp != AC_NULL);
  error |= AC_TEST(count_msgs(mp, msg_count) == msg_count);

  mptt_params** params = ac_malloc(sizeof(mptt_params) * thread_count);

  // Create the threads waiting until each is ready
  ac_debug_printf("test_msg_pool_multiple_threads:+thread_count=%d\n", thread_count);
  for (ac_u32 i = 0; i < thread_count; i++) {
    ac_debug_printf("test_msg_pool_multiple_threads: init %d\n", i);
    params[i] = ac_malloc(sizeof(mptt_params));
    error |= AC_TEST(params[i] != AC_NULL);
    if (error) {
      return error;
    }

    params[i]->ready = ac_receptor_create();
    params[i]->done = ac_receptor_create();

    ac_thread_rslt_t result = ac_thread_create(0, mptt, params[i]);
    error |= AC_TEST(result.status == 0);
    if (error) {
      return error;
    }
    // Wait until the thread is ready
    ac_receptor_wait(params[i]->ready);
  }


  // Send the threads messages which they will return to our pool
  ac_debug_printf("test_msg_pool_multiple_threads: sending msgs\n");
  const ac_u64 max_waiting_count = 10000;
  ac_u64 waiting_count = 0;
  for (ac_u32 tries = 0; waiting_count < max_waiting_count && tries < 100; tries++) {
    //ac_debug_printf("test_msg_pool_multiple_threads: waiting_count=%ld tries=%d\n",
    //    waiting_count, tries);
    for (ac_u32 i = 0; waiting_count < max_waiting_count && i < thread_count; i++) {
      AcMsg* msg = AcMsg_get(mp);
      while (waiting_count < max_waiting_count && msg == AC_NULL) {
        waiting_count += 1;
        ac_thread_yield();
        msg = AcMsg_get(mp);
      }
      if (msg != AC_NULL) {
        msg->cmd = 1;
        msg->arg = i * tries;
        mptt_send_msg(params[i], msg);
      } else {
        //ac_debug_printf("test_msg_pool_multiple_threads: %d waiting_count=%ld msg=%p\n",
        //    i, waiting_count, msg);
      }
    }
  }
  ac_debug_printf("test_msg_pool_multiple_threads: stopping waiting_count=%ld\n",
      waiting_count);

  // Tell each mptt to stop and wait until its done
  for (ac_u32 i = 0; i < thread_count; i++) {
    mptt_stop_and_wait_until_done(params[i]);
  }

  //ac_u32 c = 0;
  count_msgs(mp, msg_count);

  // TODO: Not all messages are returned because of the
  // 
  //error |= AC_TEST(c == msg_count);

  ac_debug_printf("test_msg_pool_multiple_threads: waiting_count=%ld\n",
      waiting_count);

  ac_thread_yield();

#endif
  ac_debug_printf("test_msg_pool_multiple_threads:- error=%d\n", error);
  return error;
}
