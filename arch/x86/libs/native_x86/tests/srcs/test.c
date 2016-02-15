/*
 * Copyright 2015 Wink Saville
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

#include <native_x86.h>

#include <ac_printf.h>
#include <ac_test.h>

ac_bool test_seg_regs(void) {
  ac_bool error = AC_FALSE;
  ac_u16 seg_reg1;
  ac_u16 seg_reg2;

  // Get current value and verifiy we can write it
  // and read back the same value. Not a great test
  // but in the short term anything else would be
  // fatal.
  seg_reg1 = get_ds();
  set_ds(seg_reg1);
  seg_reg2 = get_ds();

  error |= AC_TEST_EM(seg_reg1 == seg_reg2,
      "Unable to get/set/get register DS");

  seg_reg1 = get_ss();
  set_ss(seg_reg1);
  seg_reg2 = get_ss();

  error |= AC_TEST_EM(seg_reg1 == seg_reg2,
      "Unable to get/set/get register SS");

  seg_reg1 = get_es();
  set_es(seg_reg1);
  seg_reg2 = get_es();

  error |= AC_TEST_EM(seg_reg1 == seg_reg2,
      "Unable to get/set/get register ES");

  seg_reg1 = get_tr();
  //set_tr(seg_reg1); // Work's on QEMU but fails on real hardware
  seg_reg2 = get_tr();

  error |= AC_TEST_EM(seg_reg1 == seg_reg2,
      "Unable to get/set/get register TR");

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_seg_regs();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
