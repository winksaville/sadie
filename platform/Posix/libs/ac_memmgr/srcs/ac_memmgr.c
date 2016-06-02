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

#include <stdlib.h>

/**
 * Allocate size bytes
 *
 * @param: size is the number of bytes in each item
 *
 * @return: pointer to the memory
 */
void* ac_malloc(ac_size_t size) {
  if (size == 0) {
    // Standard C99 implementations may return AC_NULL or other value
    // but its undefined behavior if the returned value is used.
    // Therefore we require ac_malloc(0) to always return AC_NULL
    return AC_NULL;
  } else {
    return malloc(size);
  }
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
  if (size == 0) {
    // Standard C99 implementations may return AC_NULL or other value
    // but its undefined behavior if the returned value is used.
    // Therefore we require ac_calloc(0) to always return AC_NULL
    return AC_NULL;
  } else {
    return calloc(count, size);
  }
}

/**
 * Free memory previously allocated with ac_malloc or ac_calloc
 *
 * @param: p is the pointer to the memory as returned by ac_malloc
 *         or ac_calloc.
 */
void ac_free(void* p) {
  free(p);
}
