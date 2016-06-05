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

#include <ac_comp_mgr.h>
#include <ac_comp_mgr/tests/incs/test.h>

#include <ac_msg_pool.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

/**
 * Test we can init the component manager, create/remove a component tice.
 *
 * @return: AC_TRUE if an error
 */
ac_bool test_thread_comps(ac_u32 threads, ac_u32 comps_per_thread) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_%dx%d:+\n", threads, comps_per_thread);

  ac_debug_printf("test_%dx%d: create msg pool\n", threads, comps_per_thread);
  AcMsgPool* mp = AcMsgPool_create(threads * comps_per_thread);

  ac_debug_printf("test_%dx%d: init comp mgr\n", threads, comps_per_thread);
  ac_u32 stack_size = 0;
  AcCompMgr* cm = AcCompMgr_init(threads, comps_per_thread, stack_size);

  ac_debug_printf("test_%dx%d: first invocation of test_comps\n", threads, comps_per_thread);
  error |= AC_TEST(test_comps(cm, mp, threads * comps_per_thread) == AC_FALSE);

  ac_debug_printf("test_%dx%d: second invocation of test_comps\n", threads, comps_per_thread);
  error |= AC_TEST(test_comps(cm, mp, threads * comps_per_thread) == AC_FALSE);

  ac_debug_printf("test_%dx%d: deinit comp mgr\n", threads, comps_per_thread);
  AcCompMgr_deinit(cm);

  ac_debug_printf("test_%dx%d:-\n", threads, comps_per_thread);
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(3);
  AcReceptor_init(20);
  AcTime_init();

#if AC_PLATFORM == VersatilePB
  ac_printf("AC_PLATFORM == VersatilePB, skipping test ac_comp_mgr\n");
#else
  error|= test_thread_comps(1, 1);
  error|= test_thread_comps(1, 2);
  error|= test_thread_comps(1, 3);
  error|= test_thread_comps(2, 1);
  error|= test_thread_comps(2, 2);
  error|= test_thread_comps(2, 3);
  error|= test_thread_comps(3, 1);
  error|= test_thread_comps(3, 2);
  error|= test_thread_comps(3, 3);
#endif

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
