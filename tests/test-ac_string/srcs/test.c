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
#include <ac_memset.h>
#include <ac_test.h>

int main(void) {
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

  // Test that compison stops at a 0 character
  char a1[4] = { '1', '2', '\0', '1' };
  char a2[4] = { '1', '2', '\0', '2' };
  error |= AC_TEST(ac_strncmp(a1, a2, 4) == 0);

  // Test ac_strncpy
  char dst[32];
  ac_memset(dst, -1, sizeof(dst));
  error |= AC_TEST(ac_strncpy(dst, fourchars, 0) == dst);
  error |= AC_TEST(dst[0] == -1);
  error |= AC_TEST(ac_strncpy(dst, fourchars, 1) == dst);
  error |= AC_TEST(dst[0] == '1');
  error |= AC_TEST(dst[1] == -1);
  error |= AC_TEST(ac_strncpy(dst, fourchars, 4) == dst);
  error |= AC_TEST(dst[0] == '1');
  error |= AC_TEST(dst[1] == '2');
  error |= AC_TEST(dst[2] == '3');
  error |= AC_TEST(dst[3] == '4');
  error |= AC_TEST(dst[4] == -1);
  error |= AC_TEST(ac_strncpy(dst, fourchars, 6) == dst);
  error |= AC_TEST(dst[0] == '1');
  error |= AC_TEST(dst[1] == '2');
  error |= AC_TEST(dst[2] == '3');
  error |= AC_TEST(dst[3] == '4');
  error |= AC_TEST(dst[4] ==   0);
  error |= AC_TEST(dst[5] ==   0);
  error |= AC_TEST(dst[6] == -1);

  // Test ac_strcpy
  ac_memset(dst, -1, sizeof(dst));
  error |= AC_TEST(ac_strcpy(dst, empty) == dst);
  error |= AC_TEST(dst[0] ==   0);
  error |= AC_TEST(dst[1] ==   -1);
  error |= AC_TEST(ac_strcpy(dst, onechar) == dst);
  error |= AC_TEST(dst[0] ==   '1');
  error |= AC_TEST(dst[1] ==   0);
  error |= AC_TEST(dst[2] ==   -1);
  error |= AC_TEST(ac_strcpy(dst, fourchars) == dst);
  error |= AC_TEST(dst[0] == '1');
  error |= AC_TEST(dst[1] == '2');
  error |= AC_TEST(dst[2] == '3');
  error |= AC_TEST(dst[3] == '4');
  error |= AC_TEST(dst[4] ==   0);
  
  if (!error) {
    ac_putchar('O');
    ac_putchar('K');
    ac_putchar('\n');
  }

  return result;
}
