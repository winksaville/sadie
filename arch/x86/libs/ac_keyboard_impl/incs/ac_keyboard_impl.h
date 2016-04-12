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
 * Non blocking read of a character from keyboard
 *
 * @return -1 if no character
 */
ac_sint ac_keyboard_read_non_blocking(void);

/**
 * Initialize this module early, must be
 * called before keyboard_init
 */
void ac_keyboard_early_init(void);

/**
 * Initialize this module defining the number of
 * sensors to support.
 */
void ac_keyboard_init(void);

#endif
