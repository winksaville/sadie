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

ac_bool test_msg_pool(void) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_msg_pool:+\n");

  AcMsgPool* mp;
  AcMsg* msg;
  AcMsg* msg2;
  AC_UNUSED(msg);

  // Testing creating an empty pool returns AC_NULL
  ac_debug_printf("test_msg_pool: create a pool with NO messages\n");
  mp = AcMsgPool_create(0);
  error |= AC_TEST(mp == AC_NULL);

  // Test requesting a message from a AC_NULL pool returns AC_NULL
  ac_debug_printf("test_msg_pool: test an empty pool that AcMsg_get returns AC_NULL\n");
  msg = AcMsg_get(mp);
  error |= AC_TEST(msg == AC_NULL);

  // Test returning a AC_NULL msg to a AC_NULL pool doesn't blow up
  ac_debug_printf("test_msg_pool: returning an AC_NULL message doesn't blow up\n");
  AcMsg_ret(msg);

  // Testing creating a pool with one msg
  ac_debug_printf("test_msg_pool: create a pool with 1 mesg\n");
  mp = AcMsgPool_create(1);
  error |= AC_TEST(mp != AC_NULL);

  // Test requesting the message
  ac_debug_printf("test_msg_pool: get msg expecting != AC_NULL\n");
  msg = AcMsg_get(mp);
  error |= AC_TEST(msg != AC_NULL);

  // Test requesting the message which should be empty
  ac_debug_printf("test_msg_pool: get msg expecting AC_NULL\n");
  msg2 = AcMsg_get(mp);
  error |= AC_TEST(msg2 == AC_NULL);

  ac_debug_printf("test_msg_pool: return msg=%p\n", msg);
  // Test returning the message we successfully got
  AcMsg_ret(msg);

  // Test we can get a message back after its returned
  ac_debug_printf("test_msg_pool: test we can get a msg after returning\n");
  msg = AcMsg_get(mp);
  error |= AC_TEST(msg != AC_NULL);

  // Return msg
  AcMsg_ret(msg);

  ac_debug_printf("test_msg_pool:-\n");
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  ac_receptor_init(256);
  ac_time_init();

  ac_debug_printf("sizeof(AcMsg)=%d\n", sizeof(AcMsg));

  error |= test_msg_pool();
  error |= test_msg_pool_multiple_threads(1);
  error |= test_msg_pool_multiple_threads(10);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
