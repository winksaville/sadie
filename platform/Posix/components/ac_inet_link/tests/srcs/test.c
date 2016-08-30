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

#include <ac_inet_link.h>
#include <ac_inet_link_internal.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_thread.h>
#include <ac_test.h>

AcUint test_inet_link_impl(void) {
  AcUint error = AC_FALSE;

  return error;
}

int main(void) {
  AcUint error = AC_FALSE;

  ac_thread_init(3);
  AcReceptor_init(256);

  AcCompMgr cm;
  AcStatus status = AcCompMgr_init(&cm, 3, 10, 0);
  error |= AC_TEST(status == AC_STATUS_OK);

  AcInetLink_init(&cm);

  if (!error) {
    error |= test_inet_link_impl();
  }

  AcInetLink_deinit(&cm);

  AcCompMgr_deinit(&cm);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
