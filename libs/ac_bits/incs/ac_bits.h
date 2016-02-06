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

#define AC_BIT(type, n) ({ \
  type result = (type)(1 << n); \
  result; \
})

#define AC_BIT_MASK(type, bit_count) ({ \
  type result = ~(((type)(-1)) << (bit_count)); \
  result; \
})

#define AC_GET_BITS(type, val, bit_idx, bit_count) ({ \
  type result = ((type)val >> bit_idx) & AC_BIT_MASK(type, bit_count); \
  result; \
})

#define AC_SET_BITS(type, val, new_bits, bit_idx, bit_count) ({ \
  type mask = AC_BIT_MASK(type, bit_count); \
  type result = (type)((type)val & ~(mask << bit_idx)); \
  result |= (type)(((type)new_bits & mask) << bit_idx); \
})

#endif
