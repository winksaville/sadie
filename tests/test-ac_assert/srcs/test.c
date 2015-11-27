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

#define NDEBUG

#include <ac_inttypes.h>
#include <ac_assert.h>
#include <ac_debug_assert.h>
#include <ac_printf.h>

volatile ac_u32 volatile0 = 0;

ac_bool test_assert(void) {
    /*
     * Manually test these compile time errors by enabling both
     * and the compilation should fail.
     */
    //ac_static_assert(1 == 0, "ac_static_assert(1 == 0), should always fail");
    //ac_static_assert(volatile0 == 0);

    /*
     * Manually test these runtime errors, enable one at a time
     * and compile and run, each statement should fail.
     */
    //ac_fail("failing");
    //ac_assert(0 == 1);
    //ac_assert(volatile0 == 1);
    //ac_debug_assert(1 == 2);

    /*
     * Manually uncomment the statement below and in .config
     * set CONFIG_USER_DEBUG_BUILD=y this causes NDEBUG to NOT
     * be defined. Compile and run and you should see a runtime
     * error.
     *
     * Next set CONFIG_USER_DEBUG_BUILD=n recompile and rerun.
     * The statement will NOT fail because NDEBUG is now defined.
     * This verifies seL4_DebugAssert compiles to a NOP when
     * NDEBUG is defined.
     */
    ac_debug_assert(volatile0 == 2);

    // These should always succeed
    ac_static_assert(0 == 0, "ac_static_assert(0 == 0) should never fail");
    ac_assert(0 == 0);
    ac_assert(volatile0 == 0);

    return 0;
}

int main(void) {
    if (test_assert()) {
        // Failed
        ac_printf("ERR\n");
        return 1;
    } else {
        // Succeeded
        ac_printf("OK\n");
        return 0;
    }
}

