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

#ifndef ARCH_GENERIC_INCS_AC_XSTR_H
#define ARCH_GENERIC_INCS_AC_XSTR_H

// Stringification of macro parameters
// [See](https://gcc.gnu.org/onlinedocs/cpp/Stringification.html)

#define AC_XSTR(s) AC_STR(s)
#define AC_STR(s) #s

#endif
