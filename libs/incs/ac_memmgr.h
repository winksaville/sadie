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

/**
 * WARNING/TODO: ac_malloc/ac_calloc do NOT free memory
 */

#ifndef SADIE_LIBS_INCS_AC_MEMMGR_H
#define SADIE_LIBS_INCS_AC_MEMMGR_H

#include <ac_inttypes.h>

/**
 * Allocate size bytes
 *
 * @param: size is the number of bytes in each item
 *
 * @return: pointer to the memory
 */
void* ac_malloc(ac_size_t size);

/**
 * Allocate num_members * size bytes and initialize to 0.
 *
 * @param: count is number of items of size to create
 * @param: size is the number of bytes in each item
 *
 * @return: pointer to the items
 */
void* ac_calloc(ac_size_t num_members, ac_size_t size);

/**
 * Free memory previously allocated with ac_malloc or ac_calloc
 *
 * @param: p is the pointer to the memory as returned by ac_malloc
 *         or ac_calloc.
 */
void ac_free(void* p);

#endif
