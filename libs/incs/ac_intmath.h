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

#ifndef SADIE_LIBS_INCS_AC_INTMATH_H
#define SADIE_LIBS_INCS_AC_INTMATH_H

#include <ac_inttypes.h>

/**
 * Divide x / y and round up the result
 */
#define AC_U64_DIV_ROUND_UP(x, y) ({    \
    AcU64 rem = (x) % (y);              \
    AcU64 result = ((x) + rem) / (y);   \
    result;                             \
})

/**
 * Divide x / y and round up the result
 */
#define AC_U32_DIV_ROUND_UP(x, y) ({    \
    AcU32 rem = (x) % (y);              \
    AcU32 result = ((x) + rem) / (y);   \
    result;                             \
})

#define AC_COUNT_ONE_BITS(value) ({     \
  AcUint one_bits = 0;                  \
  AcUptr v = value;                     \
  for (AcUint i = 0; v != 0; i++) {     \
    one_bits += v & 1;                  \
    v >>= 1;                            \
  }                                     \
  one_bits;                             \
})

#endif
