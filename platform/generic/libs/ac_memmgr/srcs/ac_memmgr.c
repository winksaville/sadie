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
#include <ac_memset.h>

static ac_size_t idx = 0;

#define MEM_ALIGN ((ac_size_t)64) // MUST BE POWER of 2
#define MAX_IDX (0x1000 * 0x80)   // Assume 500K of ram

static ac_u8 mem_array[MAX_IDX] __attribute__ ((aligned (MEM_ALIGN)));

/**
 * Allocate size bytes
 *
 * @param: size is the number of bytes in each item
 *
 * @return: pointer to the memory
 */
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

/**
 * Allocate num_members * size bytes and initialize to 0.
 *
 * @param: count is number of items of size to create
 * @param: size is the number of bytes in each item
 *
 * @return: pointer to the items
 */
void* ac_calloc(ac_size_t count, ac_size_t size) {
  ac_size_t total_size = count * size;
  if (total_size == 0) {
    // Standard C99 implementations may return AC_NULL or other value
    // but its undefined behavior if the returned value is used.
    // Therefore we require ac_calloc(0) to always return AC_NULL
    return AC_NULL;
  } else {
    void* p = ac_malloc(total_size);
    ac_memset(p, 0, total_size);
    return p;
  }
}


/**
 * Free memory previously allocated with ac_malloc or ac_calloc
 *
 * @param: p is the pointer to the memory as returned by ac_malloc
 *         or ac_calloc.
 */
void ac_free(void* p) {
  /** do nothing for now */
}
