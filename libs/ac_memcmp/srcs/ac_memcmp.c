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

#include <ac_stop.h>
#include <ac_printf.h>

/**
 * Compare unsigned 8 bit bytes in memory.
 *
 * @param s1 points to the first array
 * @param s2 points to the second array
 * @param count is an unsigned integer the size of a ac_uptr
 *
 * @return = 0 if all unsigned bytes over the entire array are identical
 *         > 0 if *(ac_u8*)s1 - *(ac_u8*)s2 was positive
 *         < 0 if *(ac_u8*)s1 - *(ac_u8*)s2 was negative
 */
int ac_memcmp(const void *s1, const void* s2, ac_size_t count) {
  ac_u8* p1 = (ac_u8*)s1;
  ac_u8* p2 = (ac_u8*)s2;

  while (count > 0) {
    ac_u8 ch1 = *p1++;
    ac_u8 ch2 = *p2++;
    int result = ch1 - ch2;
    if (result != 0) {
      return result;
    }
  }
  return 0;
}

