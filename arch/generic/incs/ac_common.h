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

#ifndef SADIE_GENERIC_AC_COMMON_H
#define SADIE_GENERIC_AC_COMMON_H

#define AC_NULL ((void*)0)

#define IS_AC_NULL(ptr) (((ac_uptr)ptr) == ((ac_uptr)AC_NULL))
#define IS_NOT_AC_NULL(ptr) (((ac_uptr)ptr) != ((ac_uptr)AC_NULL))

/**
 * Boolean
 */
typedef ac_u8 ac_bool;

/**
 * Boolean false is 0
 */
#define AC_FALSE (0)

/**
 * Boolean true is 1
 */
#define AC_TRUE (1)

/**
 * Indicate to compiler an that the unused parameter is OK
 */
#define AC_UNUSED(x) (void)(x)

/**
 * AC_ARRAY_COUNT return number of elements in a array
 */
#define AC_ARRAY_COUNT(__array) (sizeof(__array)/sizeof(__typeof__(__array[0])))

#endif
