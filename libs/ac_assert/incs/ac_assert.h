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

#ifndef SADIE_LIBS_INCS_AC_ASSERT_H
#define SADIE_LIBS_INCS_AC_ASSERT_H

/**
 * Function for use by ac_fail and ac_assert.
 */
void ac_fail_impl(const char*  str, const char* file, int line, const char* function);

/**
 * Unconditionally fail writing the message to the terminal
 * plus the file, line and function printed.
 */
#define ac_fail(s) ac_fail_impl(s, __FILE__, __LINE__, __func__)

/**
 * If the expr is false at runtime _ac_fail is called with the
 * expr as a string plus the file, line and function.
 */
#define ac_assert(expr) \
    do { if (!(expr)) { ac_fail_impl(#expr, __FILE__, __LINE__, __FUNCTION__); } } while(0)

/**
 * If expr is false at compile time a compile error is generated.
 */
#define ac_static_assert(expr, message) _Static_assert(expr, message)

#endif
