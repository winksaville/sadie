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

//#define NDEBUG

#include <ac_inttypes.h>
#include <ac_assert.h>
#include <ac_debug_assert.h>
#include <ac_printf.h>
#include <ac_test.h>

volatile ac_u32 volatile0;

void ac_fail_impl(const char* assertion, const char* file, int line,
    const char* function) {
    ac_printf("Assert/Failure: '%s' at %s:%u in function %s\n",
       assertion, file, line, function);
}

int main(void) {
  volatile0 = 0;
  ac_bool error = AC_FALSE;

  /*
   * Manually test these runtime errors, enable one at a time
   * and compile and run, each statement should fail.
   */
  //ac_static_assert(1 == 0, "ac_static_assert(1 == 0), should always fail");
  //ac_static_assert(volatile0 == 0, "ac_static_assert(volatile0 == 0), should always fail");

  // Expect these asserts to fail, but since our ac_fail_impl
  // does not invoke "stop()" we can use AC_TEST to validate
  // that they failed (returned AC_TRUE) and PASS.
  error |= AC_TEST(ac_fail("failing"));
  error |= AC_TEST(ac_assert(0 == 1));
  error |= AC_TEST(ac_assert(volatile0 == 1));
  error |= AC_TEST(ac_debug_assert(1 == 2));
  error |= AC_TEST(ac_debug_assert(volatile0 == 2));

  // These should never fail
  ac_static_assert(0 == 0, "ac_static_assert(0 == 0) should never fail");
  error |= AC_TEST(!ac_assert(0 == 0));
  error |= AC_TEST(!ac_assert(volatile0 == 0));

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
