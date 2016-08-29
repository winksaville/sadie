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

#ifndef SADIE_LIBS_AC_BITS_INCS_AC_BITS_H
#define SADIE_LIBS_AC_BITS_INCS_AC_BITS_H

#include <ac_inttypes.h>

#define AC_BIT(type, n) ((type)1 << (n))

#define AC_BIT_MASK(type, bit_count) (~((type)((~(type)0) << (bit_count))))

#define AC_GET_BITS(type, val, bit_count, bit_idx) (\
  (type)(((val) >> (bit_idx)) & AC_BIT_MASK(__typeof__(val), (bit_count))) \
)

#define AC_SET_BITS(type, val, new_bits, bit_count, bit_idx) ( \
   (type)(((val) & (~(AC_BIT_MASK(__typeof__(val), (bit_count)) << (bit_idx)))) | \
     ((((__typeof__(val))(new_bits)) & AC_BIT_MASK(__typeof__(val), (bit_count))) << (bit_idx))) \
)

#define AC_GET_LOWB(val) AC_GET_BITS(ac_u8, val, 8, 0)

#define AC_GET_HIB(val) AC_GET_BITS(ac_u8, val, 8, 8)

#endif
