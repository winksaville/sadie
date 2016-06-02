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

#include <ac_comp_mgr.h>
#include <ac_comp_mgr/tests/incs/test.h>

#include <ac_msg_pool.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  ac_receptor_init(256);
  AcTime_init();

#if AC_PLATFORM == VersatilePB
  ac_printf("AC_PLATFORM == VersatilePB, skipping test ac_comp_mgr\n");
#else
  error|= test_1x1();
#endif

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
