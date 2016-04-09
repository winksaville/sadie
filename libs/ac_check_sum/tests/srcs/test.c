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

#include <ac_check_sum.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>


int main(void) {
  ac_bool error = AC_FALSE;

  ac_u8 array0[] = {};
  ac_u8 array1[] = {1};
  ac_u8 array2[] = {1,1};
  ac_u8 array3[] = {1,0,0xfe};
  ac_u8 array4[] = {1,1,0xfe};
  ac_u8 array5[] = {1,2,0xfe};

  error |= AC_TEST(ac_check_sum_u8(array0, sizeof(array0)) == 0);
  error |= AC_TEST(ac_check_sum_u8(array1, sizeof(array1)) == 1);
  error |= AC_TEST(ac_check_sum_u8(array2, sizeof(array2)) == 2);
  error |= AC_TEST(ac_check_sum_u8(array3, sizeof(array3)) == 0xff);
  error |= AC_TEST(ac_check_sum_u8(array4, sizeof(array4)) == 0x00);
  error |= AC_TEST(ac_check_sum_u8(array5, sizeof(array5)) == 0x01);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
