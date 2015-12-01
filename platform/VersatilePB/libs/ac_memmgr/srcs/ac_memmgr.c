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

#include <ac_memmgr.h>

static ac_size_t idx = 0;

#define MEM_ALIGN ((ac_size_t)64) // MUST BE POWER of 2
#define MAX_IDX (0x1000 * 64)

static ac_u8 mem_array[MAX_IDX] __attribute__ ((aligned (MEM_ALIGN)));

/** For the moment we allocate but don't free */
void* ac_malloc(ac_size_t size) {
  ac_size_t cur_idx;
  ac_size_t next_idx;
  ac_bool ok;

  // We must return AC_NULL if size is 0
  if (size == 0) {
    return AC_NULL;
  }

  // Roundup to next alignment factor
  size = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);

  // Loop until we can allocate or we have no more memory
  do {
    cur_idx = __atomic_load_n(&idx, __ATOMIC_ACQUIRE);

    next_idx = cur_idx + size;
    if ((next_idx >= MAX_IDX) || (next_idx <= cur_idx)) {
      return AC_NULL;
    }
    ok = __atomic_compare_exchange_n(&idx, &idx, next_idx,
        AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
  } while (!ok);

  return &mem_array[cur_idx];
}

void ac_free(void* p) {
  /** do nothing for now */
}
