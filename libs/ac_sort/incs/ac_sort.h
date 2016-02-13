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

#ifndef SADIE_LIBS_AC_MEMCMP_INCS_AC_MEMCMP_H
#define SADIE_LIBS_AC_MEMCMP_INCS_AC_MEMCMP_H

#include <ac_inttypes.h>

/**
 * Compare list[idx1] and list[idx2]
 *
 * @return = 0 if equal
 *         > 0 if e1 > e2
 *         < 0 if e1 < e2
 */
typedef ac_sint (*compare_by_idx_fn)(const void* list, const ac_uint idx1,
    const ac_uint idx2);

/** Swap two elements */
typedef void (*swap_by_idx_fn)(void* list, const ac_uint idx1,
    const ac_uint idx2);

/**
 * Sort a randomly iterable list in "ascending" order
 * based on compare.
 */
static __inline__ void ac_sort_rand_iter(void* list, ac_uint count,
    compare_by_idx_fn compare, swap_by_idx_fn swap) {
  
  // Simple bubble sort for now
  for (ac_uint o_idx = 0; o_idx < count; o_idx++) {
    for (ac_uint i_idx = o_idx+1; i_idx < count; i_idx++) {
      ac_sint cmp = compare(list, o_idx, i_idx);
      if (cmp > 0) {
        // o_el > i_el
        swap(list, o_idx, i_idx);
      }
    }
  }
}

#endif
