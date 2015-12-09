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
 * This module implements ac_assert and is like assert but
 * is testable because the failure code, ac_fail_impl is not
 * required to stop and the ac_assert macro will return AC_TRUE
 * if it did invoke ac_fail_impl.
 *
 * See test-ac_assert for examples.
 */

#ifndef SADIE_LIBS_INCS_AC_ASSERT_H
#define SADIE_LIBS_INCS_AC_ASSERT_H

#include <ac_inttypes.h>

/**
 * Function for use by ac_fail and ac_assert. This function can
 * be overridden but the default implemenaton will print the
 * parameters and invoke ac_stop().
 */
void ac_fail_impl(const char*  str, const char* file, int line, const char* function);

/**
 * Unconditionally fail writing the message to the terminal
 * plus the file, line and function printed and always
 * return AC_TRUE indicating there was a failure.
 */
#define ac_fail(s) ({ ac_fail_impl(s, __FILE__, __LINE__, __func__); AC_TRUE; })

/**
 * If the expr is false at runtime ac_fail is called with the
 * expr as a string and AC_TRUE is returned indicating a failure.
 */
#define ac_assert(expr) ({ \
  ac_bool failed;          \
  if (expr) {              \
    failed = AC_FALSE;     \
  } else {                 \
    ac_fail(#expr);        \
    failed = AC_TRUE;      \
  }                        \
  failed;                  \
})

//#define ac_assert(expr) expr

/**
 * If expr is false at compile time a compile error is generated.
 */
#define ac_static_assert(expr, message) _Static_assert(expr, message)

#endif
