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
 * Defines s8 ... u64 assuming sizeof long is 64 bits
 */
#ifndef SADIE_GENERIC_AC_INTTYPES_l64_H
#define SADIE_GENERIC_AC_INTTYPES_l64_H

typedef signed char AcS8;
typedef signed short AcS16;
typedef signed int AcS32;
typedef signed long AcS64;

typedef unsigned char AcU8;
typedef unsigned short AcU16;
typedef unsigned int AcU32;
typedef unsigned long AcU64;

typedef signed char ac_s8;
typedef signed short ac_s16;
typedef signed int ac_s32;
typedef signed long ac_s64;

typedef unsigned char ac_u8;
typedef unsigned short ac_u16;
typedef unsigned int ac_u32;
typedef unsigned long ac_u64;

#endif
