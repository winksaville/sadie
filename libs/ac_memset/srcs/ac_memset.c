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
#include <ac_thread.h>

/**
 * Set memory to the given byte value
 *
 * @param s points to the array
 * @param val is a byte used to fill the array
 * @param count is an integer the size of a pointer
 *
 * @return = s
 */
void* ac_memset(void *s, ac_u8 val, ac_uptr count) {
  // TODO: Optimize
  ac_u8* pU8 = s;
  for(ac_uptr i = 0; i < count; i++) {
    *pU8++ = val;
  }
  return s;
}

#if AC_PLATFORM != Posix
/**
 * Set memory, compiler needs this for initializing structs and such.
 */
void* memset(void *s, int val, unsigned long count) {
  return ac_memset(s, val, count);
}
#endif
