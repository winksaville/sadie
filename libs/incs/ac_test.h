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
 * Test the condition and return AC_FALSE if cond is "true" else return AC_TRUE
 */
#define AC_TEST(cond, text) ({                                          \
  ac_bool result = (cond) ? AC_FALSE : AC_TRUE;                         \
  if (result) {                                                         \
    ac_printf("Error %s:%d FN %s: %s. Condition '" #cond "' failed.\n", \
           __FILE__, __LINE__, __func__, text);                         \
  }                                                                     \
  result;                                                               \
})

#endif
