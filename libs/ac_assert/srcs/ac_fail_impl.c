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

#include <ac_stop.h>
#include <ac_printf.h>

/**
 * Invoked by macro ac_assert in ac_assert.h
 */
void ac_fail_impl(const char* assertion, const char* file, int line, const char* function) {
    ac_printf("Assert/Failure: '%s' at %s:%u in function %s\n",
       assertion, file, line, function);
    ac_stop();
}
