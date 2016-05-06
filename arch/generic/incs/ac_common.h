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
typedef _Bool ac_bool;

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

/**
 * Cast x to ac_uint
 */
#define AC_UINT(x) ({ ((ac_uint)(x)); })

/**
 * Cast x to ac_sint
 */
#define AC_SINT(x) ({ ((ac_sint)(x)); })

/**
 * Cast x to ac_int
 */
#define AC_INT(x) ({ ((ac_int)(x)); })


/**
 * Define MAX/MIN's
 */
#define AC_U64_MAX ((ac_u64)0xFFFFFFFFFFFFFFFFll)
#define AC_U64_MIN ((ac_u64)0x0ll)
#define AC_S64_MAX ((ac_u64)0x7FFFFFFFFFFFFFFFll)
#define AC_S64_MIN ((ac_s64)(0xFFFFFFFFFFFFFFFFll))

#define AC_U32_MAX ((ac_u32)0xFFFFFFFF)
#define AC_U32_MIN ((ac_u32)0x0)
#define AC_S32_MAX ((ac_s32)0x7FFFFFFF)
#define AC_S32_MIN ((ac_s32)0xFFFFFFFF)

#define AC_U16_MAX ((ac_u16)0xFFFF)
#define AC_U16_MIN ((ac_u16)0x0ll)
#define AC_S16_MAX ((ac_s16)0x7FFF)
#define AC_S16_MIN ((ac_s16)0xFFFF)

#define AC_U8_MAX ((ac_u8)0xFF)
#define AC_U8_MIN ((ac_u8)0x0ll)
#define AC_S8_MAX ((ac_s8)0x7F)
#define AC_S8_MIN ((ac_s8)0xFF)

#ifndef AC_SIZEOF_INT

#error "AC_SIZEOF_INT isn't defined"

#endif

#define AC_UINT_SIZE AC_SIZEOF_INT
#define AC_SINT_SIZE AC_SIZEOF_INT
#define AC_INT_SIZE AC_SIZEOF_INT


#if AC_SIZEOF_INT == 8

#define AC_UINT_MAX AC_U64_MAX
#define AC_UINT_MIN AC_U64_MIN
#define AC_SINT_MAX AC_S64_MAX
#define AC_SINT_MIN AC_S64_MIN

#elif AC_SIZEOF_INT == 4

#define AC_UINT_MAX AC_U32_MAX
#define AC_UINT_MIN AC_U32_MIN
#define AC_SINT_MAX AC_S32_MAX
#define AC_SINT_MIN AC_S32_MIN

#else

#error "ERROR AC_SIZEOF_INT isn't 8 or 4"

#endif

#define AC_INT_MAX AC_SINT_MAX
#define AC_INT_MIN AC_SINT_MIN

#endif
