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

#include <ac_string.h>
#include <ac_putchar.h>
#include <ac_test.h>

int main(void) {
  ac_size_t ret;
  char* empty = "";
  char* onechar = "1";
  char* fourchars = "1234";
  int result = 0;
  ac_bool error = AC_FALSE;
  
  error |= AC_TEST(ac_strlen(empty) == 0);
  error |= AC_TEST(ac_strlen(onechar) == 1);
  
  error |= AC_TEST(ac_strlen(fourchars) == 4);
  error |= AC_TEST(ac_strncmp(empty, empty, 0) == 0);
  error |= AC_TEST(ac_strncmp(empty, onechar, 1) < 0);
  error |= AC_TEST(ac_strncmp(onechar, empty, 1) > 0);
  error |= AC_TEST(ac_strncmp(onechar, onechar, 1) == 0);
  error |= AC_TEST(ac_strncmp(onechar, fourchars, 4) < 0);
  error |= AC_TEST(ac_strncmp(fourchars, onechar, 4) > 0);
  error |= AC_TEST(ac_strncmp(fourchars, fourchars, 4) == 0);

  if (!error) {
    ac_putchar('O');
    ac_putchar('K');
    ac_putchar('\n');
  }

  return result;
}
