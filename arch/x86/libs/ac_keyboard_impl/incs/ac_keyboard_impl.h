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

#ifndef SADIE_ARCH_X86_LIBS_AC_KEYBOARD_IMPL_INCS_AC_KEYBOARD_IMPL_H
#define SADIE_ARCH_X86_LIBS_AC_KEYBOARD_IMPL_INCS_AC_KEYBOARD_IMPL_H

#include <ac_inttypes.h>

/**
 * Read of a character from keyboard, block if none available
 *
 * @return character
 */
ac_u8 ac_keyboard_rd(void);

/**
 * Read keyboard status
 *
 * @return AC_TRUE if character is ready
 */
ac_bool ac_keyboard_rd_status(void);

/**
 * Initialize this module early, must be
 * called before keyboard_init
 */
void ac_keyboard_early_init(void);

#endif
