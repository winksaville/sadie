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

#ifndef SADIE_LIBS_INCS_AC_TEST_H
#define SADIE_LIBS_INCS_AC_TEST_H

#include <ac_inttypes.h>
#include <ac_printf.h>

/**
 * If expr is false then print an "ERROR: '#expr' ..." and return AC_TRUE
 * else return AC_FALSE and no error is printed.
 */
#define AC_TEST(expr) ({                                         \
  ac_bool __result__ = (expr) ? AC_FALSE : AC_TRUE;              \
  if (__result__) {                                              \
    ac_printf("ERROR: expr '" #expr "' failed in %s:%d FN %s\n", \
           __FILE__, __LINE__, __func__);                        \
  }                                                              \
  __result__;                                                    \
})

/**
 * If expr is false then print an "ERROR: 'errstr' ..." and return AC_TRUE
 * else return AC_FALSE and no error is printed.
 */
#define AC_TEST_EM(expr, err_msg) ({                             \
  ac_bool __result__ = (expr) ? AC_FALSE : AC_TRUE;              \
  if (__result__) {                                              \
    ac_printf("ERROR: '" err_msg "' in %s:%d FN %s\n",           \
           __FILE__, __LINE__, __func__);                        \
  }                                                              \
  __result__;                                                    \
})

#endif
