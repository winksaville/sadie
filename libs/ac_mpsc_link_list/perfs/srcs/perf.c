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

#include <ac_mpsc_link_list.h>
#include <ac_mpsc_link_list_dbg.h>

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_message.h>
#include <ac_message_pool.h>
#include <ac_receptor.h>
#include <ac_status.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>
#include <ac_thread.h>

AcBool simple_mpsc_link_list_perf(AcU64 loops) {
  AcStatus status;
  AcBool error = AC_FALSE;
  ac_debug_printf("simple_mpsc_link_list_perf:+loops=%lu\n", loops);

  AcMpscLinkList list;
  AcMessagePool pool;

  ac_u32 count = 2;
  ac_u32 data_size = 2;

  status = AcMessagePool_init(&pool, count, data_size);
  ac_printf("simple_mpsc_link_list_perf: allocate pool status=%d\n", status);
  if (status != AC_STATUS_OK) {
    error |= AC_TRUE;
    goto done;
  }

  status = AcMpscLinkList_init(&list);
  ac_printf("simple_mpsc_link_list_perf: allocate list status=%d\n", status);
  if (status != AC_STATUS_OK) {
    error |= AC_TRUE;
    goto done;
  }

  AcMessage* msg = AcMessagePool_get_msg(&pool);
  AcU64 start = ac_tscrd();

  for (AcU64 i = 0; i < loops; i++) {
    AcMpscLinkList_add(&list, msg);
    msg = AcMpscLinkList_rmv(&list);
    ac_debug_printf("%d msg=%p\n", i, msg);
  }

  AcU64 stop = ac_tscrd();
  AcMessagePool_ret_msg(msg);

  AcU64 duration = stop - start;
  AcU64 ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_link_list_perf: empty     duration=%lu time=%.9t ns_per_op=%.3Sns\n",
      duration, duration, ns_per_op);


  // Add one message to the list is not empty
  AcMessage* msg1 = AcMessagePool_get_msg(&pool);
  AcMpscLinkList_add(&list, msg1);

  // Get another one to add to the non-empty list
  msg = AcMessagePool_get_msg(&pool);
  start = ac_tscrd();

  for (AcU64 i = 0; i < loops; i++) {
    AcMpscLinkList_add(&list, msg);
    msg = AcMpscLinkList_rmv(&list);
    ac_debug_printf("%d msg=%p\n", i, msg);
  }

  stop = ac_tscrd();
  AcMessagePool_ret_msg(msg);

  duration = stop - start;
  ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_link_list_perf: non-empty duration=%lu time=%.9t ns_per_op=%.3Sns\n",
      duration, duration, ns_per_op);

  // Return the that still on the list and return to the pool
  msg1 = AcMpscLinkList_rmv(&list);
  AcMessagePool_ret_msg(msg1);

  AcMpscLinkList_deinit(&list);
  AcMessagePool_deinit(&pool);

 done:

  ac_printf("simple_mpsc_link_list_perf:-\n");
  return error;
}

/**
 * main
 */
int main(void) {
  AcBool error = AC_FALSE;

  ac_thread_init(10);
  AcReceptor_init(50);
  AcTime_init();

  ac_debug_printf("sizeof(AcMem)=%d\n", sizeof(AcMem));

  error |= simple_mpsc_link_list_perf(200000000);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
