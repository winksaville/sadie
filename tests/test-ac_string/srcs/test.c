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

int main(void) {
  char* empty = "";
  char* onechar = "1";
  char* fourchars = "1234";
  int result = 0;
  ac_size_t ret;
  
  ret = ac_strlen(empty);
  if (ret != 0) {
    result |= 0x01;
  }

  ret = ac_strlen(onechar);
  if (ret != 1) {
    result |= 0x02;
  }

  ret = ac_strlen(fourchars);
  if (ret != 4) {
    result |= 0x04;
  }

  ret = ac_strncmp(empty, empty, 0);
  if (ret != 0) {
    result |= 0x08;
  }

  ret = ac_strncmp(empty, onechar, 1);
  if (ret >= 0) {
    result |= 0x10;
  }

  ret = ac_strncmp(onechar, empty, 1);
  if (ret <= 0) {
    result |= 0x20;
  }

  ret = ac_strncmp(onechar, onechar, 1);
  if (ret != 0) {
    result |= 0x40;
  }

  ret = ac_strncmp(onechar, fourchars, 4);
  if (ret >= 0) {
    result |= 0x80;
  }

  ret = ac_strncmp(fourchars, onechar, 4);
  if (ret <= 0) {
    result |= 0x100;
  }

  ret = ac_strncmp(fourchars, fourchars, 4);
  if (ret != 0) {
    result |= 0x200;
  }

  if (result == 0) {
    ac_putchar('O');
    ac_putchar('K');
    ac_putchar('\n');
  } else {
    ac_putchar('E');
    ac_putchar('R');
    ac_putchar('R');
    ac_putchar('\n');
  }

  return result;
}
