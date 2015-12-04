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

void ac_exception_reset_handler(void) __attribute__ ((interrupt ("UNDEF")));
void ac_exception_undef_handler(void) __attribute__ ((interrupt ("UNDEF")));
void ac_exception_svc_handler(void) __attribute__ ((interrupt ("SWI")));
void ac_exception_prefetch_abort_handler(void) __attribute__ ((interrupt ("ABORT")));
void ac_exception_data_abort_handler(void) __attribute__ ((interrupt ("ABORT")));
void ac_exception_irq_handler(void) __attribute__ ((interrupt ("IRQ")));
void ac_exception_fiq_handler(void) __attribute__ ((interrupt ("FIQ")));

#endif

