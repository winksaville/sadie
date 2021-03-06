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

#ifndef SADIE_LIBS_AC_MEMCPY_INCS_AC_MEMCPY_H
#define SADIE_LIBS_AC_MEMCPY_INCS_AC_MEMCPY_H

#include <ac_inttypes.h>

/**
 * copy unsigned 8 bit bytes from src to dest
 *
 * @param dest points to the destination array
 * @param src points to the source array
 * @param count number of bytes to copy
 *
 * @return dest
 */
void* ac_memcpy(void* dest, const void* src, ac_size_t count);

#endif
