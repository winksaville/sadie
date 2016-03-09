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

#include <ac_tsc.h>

#include <ac_test.h>

ac_bool test_ac_tsc() {
  ac_bool error = AC_FALSE;

#if defined(Posix) || defined(pc_x86_64)

  ac_u64 tsc1 = ac_tscrd();
  ac_u64 tsc2 = ac_tscrd();
  ac_u64 diff = tsc2 - tsc1;
  error |= AC_TEST(tsc1 != 0);
  error |= AC_TEST(tsc2 != 0);
  error |= AC_TEST(diff > 0);
  ac_printf("test_ac_tsc: tsc1=%llx tsc2=%llx diff=%llx\n",
      tsc1, tsc2, diff);

  ac_u32 freq = ac_tsc_freq();
  error |= AC_TEST(freq >= 1000000);
  ac_printf("test_ac_tsc: freq=%d\n", freq);

#elif defined(VersatilePB)

  ac_printf("test_ac_tsc: VersatilePB no tests on qemu"
      " as Cycle Counter is not available\n");

#else

  ac_printf("test_ac_tsc: Unknown Platform\n");

#endif

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_ac_tsc();

  if (!error) {
    ac_printf("OK\n");
  }

  return 0;
}
