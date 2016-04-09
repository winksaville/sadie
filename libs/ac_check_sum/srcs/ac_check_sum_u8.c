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

#include <ac_check_sum.h>

#include <ac_inttypes.h>

/**
 * Calcuate a unsigned byte check sum.
 *
 * @param p is pointer to the data
 * @param len is number of bytes.
 * @return sum of the bytes
 */
ac_u8 ac_check_sum_u8(void *p, ac_size_t len) {
  ac_u8* pu8 = (ac_u8*)p;
  ac_u8 sum = 0;
  for (ac_size_t i = 0; i < len; i++) {
    sum += *pu8++;
  }
  return sum;
}
