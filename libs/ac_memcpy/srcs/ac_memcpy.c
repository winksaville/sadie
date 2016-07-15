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

#include <ac_memcpy.h>

#include <ac_inttypes.h>

/**
 * @see ac_memcpy.h 
 */
void* ac_memcpy(void *dest, const void* src, ac_size_t count) {
  ac_u8* pd = dest;
  const ac_u8* ps = src;
  while (count-- != 0) {
    *pd++ = *ps++;
  }
  return dest;
}


#if AC_PLATFORM == VersatilePB
void* memcpy(void *dest, const void* src, ac_size_t count) {
  ac_memcpy(dest, src, count);
  return dest;
}
#endif
