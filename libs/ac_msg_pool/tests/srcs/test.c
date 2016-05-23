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

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_test.h>

ac_bool test_msg_pool(void) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_msg_pool:+\n");

  AcMsgPool mp;
  AcMsg* msg;
  AcMsg* msg2;
  AC_UNUSED(msg);

  // Testing creating an empty pool returns AC_NULL
  mp = AcMsgPool_create(0);
  error |= AC_TEST(mp == AC_NULL);

  // Test requesting a message from a AC_NULL pool returns AC_NULL
  msg = AcMsg_get(mp);
  error |= AC_TEST(msg == AC_NULL);

  // Test returning a AC_NULL msg to a AC_NULL pool doesn't blow up
  AcMsg_ret(msg);

  // Testing creating a pool with one msg
  mp = AcMsgPool_create(1);
  error |= AC_TEST(mp != AC_NULL);

  // Test requesting the message
  msg = AcMsg_get(mp);
  error |= AC_TEST(msg != AC_NULL);

  // Test requesting the message
  msg2 = AcMsg_get(mp);
  error |= AC_TEST(msg2 == AC_NULL);

  // Test returning the message
  AcMsg_ret(msg);

  ac_debug_printf("test_msg_pool:-\n");
  return error;
}


int main(void) {
  ac_bool error = AC_FALSE;

  ac_debug_printf("sizeof(AcMsg)=%d\n", sizeof(AcMsg));

  error |= test_msg_pool();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
