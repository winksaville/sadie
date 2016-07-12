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

#ifndef SADIE_LIBS_INCS_AC_CRASH_H
#define SADIE_LIBS_INCS_AC_CRASH_H

/**
 * Used to unconditionally cause the running system/applicat
 * to stop in a non-graceful manner. Should only be use for
 * debugging.
 */
#define AC_CRASH() do { (*(volatile ac_u8*)0) = 0; } while(AC_FALSE)

#endif
