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

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>


ac_bool test_AcCompMgr(void) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_ac_comp_mgr:+\n");

  const ac_u32 max_component_threads = 3;
  const ac_u32 max_components = max_component_threads * 10;

  AcCompMgr_init(max_component_threads, max_components, 0);

  ac_debug_printf("test_ac_comp_mgr:-\n");
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  ac_receptor_init(256);
  AcTime_init();

  ac_debug_printf("sizeof(AcMsg)=%d\n", sizeof(AcMsg));

  error |= test_AcCompMgr();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
