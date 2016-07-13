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

ac_bool simple_msg_pool_test(void) {
  ac_bool error = AC_FALSE;
  AcStatus status;
  ac_debug_printf("test_msg_pool:+\n");

  AcMsgPool mp;
  AcMsg* msg;
  AcMsg* msg2;
  AC_UNUSED(msg);

  error |= AC_TEST(AC_STATUS_OK == 0);

  // Testing creating an empty pool returns AC_NULL
  ac_debug_printf("test_msg_pool: init a pool with NO messages\n");
  status = AcMsgPool_init(&mp, 0);
  error |= AC_TEST(status == AC_STATUS_OK);
  error |= AC_TEST(mp.mem_pool != AC_NULL);

  // Test requesting a message from a AC_NULL pool returns AC_NULL
  ac_debug_printf("test_msg_pool: test an empty pool that AcMsg_get returns AC_NULL\n");
  status = AcMsgPool_get_msg(&mp, &msg);
  error |= AC_TEST(status != AC_STATUS_OK);
  error |= AC_TEST(msg == AC_NULL);

  // Test returning a AC_NULL msg to a AC_NULL pool doesn't blow up
  ac_debug_printf("test_msg_pool: returning an AC_NULL message doesn't blow up\n");
  AcMsgPool_ret_msg(msg);


  // Testing creating a pool with one msg
  ac_debug_printf("test_msg_pool: init a pool with 1 mesg\n");
  status = AcMsgPool_init(&mp, 1);
  error |= AC_TEST(status == AC_STATUS_OK);
  error |= AC_TEST(mp.mem_pool != AC_NULL);

  // Test requesting the message
  ac_debug_printf("test_msg_pool: get msg expecting != AC_NULL\n");
  status = AcMsgPool_get_msg(&mp, &msg);
  error |= AC_TEST(status == AC_STATUS_OK);
  error |= AC_TEST(msg != AC_NULL);

  // Test requesting the message which should be empty
  ac_debug_printf("test_msg_pool: get msg expecting AC_NULL\n");
  status = AcMsgPool_get_msg(&mp, &msg2);
  error |= AC_TEST(status != AC_STATUS_OK);
  error |= AC_TEST(msg2 == AC_NULL);

  ac_debug_printf("test_msg_pool: return msg=%p\n", msg);
  // Test returning the message we successfully got
  AcMsgPool_ret_msg(msg);

  // Test we can get a message back after its returned
  ac_debug_printf("test_msg_pool: test we can get a msg after returning\n");
  status = AcMsgPool_get_msg(&mp, &msg);
  error |= AC_TEST(status == AC_STATUS_OK);
  error |= AC_TEST(msg != AC_NULL);

  // Return msg
  AcMsgPool_ret_msg(msg);

  // Deini the pool
  AcMsgPool_deinit(&mp);

  ac_debug_printf("test_msg_pool:-\n");
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  AcReceptor_init(256);
  AcTime_init();

  ac_debug_printf("sizeof(AcMsg)=%d\n", sizeof(AcMsg));

  error |= simple_msg_pool_test();
  //error |= test_msg_pool_multiple_threads(1, 1);
  //error |= test_msg_pool_multiple_threads(1, 10);
  //error |= test_msg_pool_multiple_threads(10, 1);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
