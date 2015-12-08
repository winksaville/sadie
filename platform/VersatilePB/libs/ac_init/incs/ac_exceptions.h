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

#ifndef SADIE_PLATFORM_VERSATILEPB_LIBS_AC_INIT_INCS_AC_EXCEPTIONS_H
#define SADIE_PLATFORM_VERSATILEPB_LIBS_AC_INIT_INCS_AC_EXCEPTIONS_H

#include <ac_inttypes.h>

typedef void (*int_handler)(void* param);

/**
 * Register a irq handler and its parameter.
 *
 * return 0 if OK
 */
ac_u32 ac_exception_irq_register(int_handler handler, void* param);

/**
 * Initialize this module
 */
void ac_exception_init();

#endif

