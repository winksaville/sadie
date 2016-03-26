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

#ifndef SADIE_ARCH_X86_LIBS_AC_THREAD_IMPL_INCS_THREAD_X86_DEBUG_CTX_SWITCH_H
#define SADIE_ARCH_X86_LIBS_AC_THREAD_IMPL_INCS_THREAD_X86_DEBUG_CTX_SWITCH_H

/**
 * Uncomment THREAD_X86_DEBUG_CTX_SWITCH below to debug context switching and
 * enable * ac_printf's in thread_yield, reschedule_isr and timer_reschedule_isr.
 *
 * Adding ac_printf's in isr causes the compiler to save rbp as the
 * first statement in the isr. Enabling THREAD_X86_DEBUG_CTX_SWITCH
 * modifies the code to properly handle this.
 *
 * I filed a bug in gcc, but I couldn't convince H.J Lu to allow the
 * programmer to somehow control saving/restoring of rbp.
 * See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70220
 */
//#define THREAD_X86_DEBUG_CTX_SWITCH

#endif
