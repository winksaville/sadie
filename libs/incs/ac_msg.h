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

#ifndef SADIE_LIBS_INCS_AC_MSG_H
#define SADIE_LIBS_INCS_AC_MSG_H

#include <ac_inttypes.h>

/**
 * A AC Message
 */
typedef struct AcMsg {
  ac_u64 arg1;          ///< argument1
  ac_u64 arg2;          ///< argument2
} AcMsg;

#endif
