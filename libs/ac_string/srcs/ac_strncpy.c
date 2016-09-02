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

#include <ac_string.h>

/**
 * Copy up to num characters from src to dst including a 0 terminator
 * if encountered in src. The dst will padded with zero's up to &dst[num-1]
 * if src < num characters in length. If src >= num characters long
 * then there will be no 0 termination in dst.
 *
 * dst and srch shall not overlap.
 *
 * @return dst
 */
char* ac_strncpy(char *dst, const char *src, ac_size_t num) {
  if (num > 0) {
    AcUint i;
    for (i = 0; i < num; i++) {
      char ch = src[i];
      dst[i] = ch;
      if (ch == 0) {
        break;
      }
    }
    for (; i < num; i++) {
      dst[i] = 0;
    }
  }
  return dst;
}
