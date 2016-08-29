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

#include <ac_msg_pool.h>
#include <ac_msg_pool/tests/incs/test.h>

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

AcBool simple_message_pool_test(void) {
  AcBool error = AC_FALSE;
  AcStatus status;
  ac_debug_printf("test_message_pool:+\n");

  AcMsgPool mp;
  AcMsg* msg;
  AcMsg* msg2;
  AcMsg* msg3;

  error |= AC_TEST(AC_STATUS_OK == 0);

  // Testing creating an empty pool returns an error
  ac_debug_printf("test_message_pool: init passing AC_NULL pointer returns error\n");
  status = AcMsgPool_init(AC_NULL, 0, 0);
  error |= AC_TEST(status != AC_STATUS_OK);

  // Testing no messages fails
  ac_debug_printf("test_message_pool: init a pool with NO messages fails\n");
  status = AcMsgPool_init(&mp, 0, 0);
  error |= AC_TEST(status != AC_STATUS_OK);

  // Testing no non-power of 2 fails
  ac_debug_printf("test_message_pool: init a pool with msg_count != power 2 in size fails\n");
  status = AcMsgPool_init(&mp, 3, 0);
  error |= AC_TEST(status != AC_STATUS_OK);

  // Testing creating a pool with 2 msgs succeeds
  ac_debug_printf("test_message_pool: init a pool with 2 messages\n");
  status = AcMsgPool_init(&mp, 2, 0);
  error |= AC_TEST(status == AC_STATUS_OK);

  // Test we can deinit
  ac_debug_printf("test_message_pool: deinit\n");
  AcMsgPool_deinit(&mp);

  // Testing recreating a pool with 2 msgs and non-zero data succeeds
  ac_debug_printf("test_message_pool: init a pool with 2 messages\n");
  status = AcMsgPool_init(&mp, 2, 1);
  error |= AC_TEST(status == AC_STATUS_OK);

  // Test returning a AC_NULL msg to a AC_NULL pool doesn't blow up
  ac_debug_printf("test_message_pool: returning an AC_NULL msg doesn't blow up\n");
  AcMsgPool_ret_msg(AC_NULL);

  // Test requesting the first msg
  ac_debug_printf("test_message_pool: get msg expecting != AC_NULL\n");
  msg = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg != AC_NULL);
  if (msg != AC_NULL) {
    error |= AC_TEST(msg->hdr.len_data == 1);
  }
  msg->data[0] = 1;
  error |= AC_TEST(msg->data[0] == 1);

  // Test requesting the first msg
  ac_debug_printf("test_message_pool: get msg2 expecting != AC_NULL\n");
  msg2 = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg2 != AC_NULL);
  if (msg2 != AC_NULL) {
    error |= AC_TEST(msg2->hdr.len_data == 1);
  }
  msg2->data[0] = 2;
  error |= AC_TEST(msg2->data[0] == 2);

  // Test requesting the msg which should be empty
  ac_debug_printf("test_message_pool: get msg expecting AC_NULL\n");
  msg3 = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg3 == AC_NULL);

  // Test returning the msg we successfully got
  ac_debug_printf("test_message_pool: return msg=%p\n", msg);
  AcMsgPool_ret_msg(msg);
  ac_debug_printf("test_message_pool: return msg2=%p\n", msg2);
  AcMsgPool_ret_msg(msg2);

  // Test we can get a msg back after its returned
  ac_debug_printf("test_message_pool: test we can get a msg after returning\n");
  msg = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg != AC_NULL);
  if (msg != AC_NULL) {
    error |= AC_TEST(msg->hdr.len_data == 1);
  }

  // Return msg
  AcMsgPool_ret_msg(msg);

  // Deinit the pool
  AcMsgPool_deinit(&mp);

  ac_debug_printf("test_message_pool:-\n");
  return error;
}

int main(void) {
  AcBool error = AC_FALSE;

  ac_thread_init(10);
  AcReceptor_init(256);
  AcTime_init();

  ac_debug_printf("sizeof(AcMsg)=%d\n", sizeof(AcMsg));

  error |= simple_message_pool_test();
  //error |= test_message_pool_multiple_threads(1, 1);
  //error |= test_message_pool_multiple_threads(1, 10);
  //error |= test_message_pool_multiple_threads(10, 1);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
