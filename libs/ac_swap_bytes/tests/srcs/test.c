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

#include <ac_swap_bytes.h>

#include <ac_test.h>
#include <ac_printf.h>

ac_bool test_swap_bytes() {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(AC_SWAP_BYTES_U16(0x0123) == 0x2301);
  error |= AC_TEST(AC_SWAP_BYTES_U32(0x01234567) == 0x67452301);
  error |= AC_TEST(AC_SWAP_BYTES_U64(0x0123456789ABCDEF) == 0xEFCDAB8967452301);

  return error;
}


int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_swap_bytes();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
