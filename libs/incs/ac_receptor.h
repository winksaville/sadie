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

#ifndef SADIE_LIBS_INCS_AC_RECEPTOR_H
#define SADIE_LIBS_INCS_AC_RECEPTOR_h

/**
 * Receptor structure
 */
//typedef ac_receptor_t;

// The following will be defined in ac_receptor_impl.h

/**
 * Create a receptor and set its state to NOT signaled
 *
 * @return AC_NULL if failed.
 */
//ac_receptor_t ac_receptor_create(void);

/**
 * Wait for the receptor to be signaled only one entity can wait
 * on a receptor at a time. If the receptor has already been signaled
 * ac_receptor_wait will return immediately.
 *
 * @return 0 if successfull !0 indicates an error
 */
//ac_uint ac_receptor_wait(ac_receptor_t receptor);

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 */
//void ac_receptor_signal(ac_receptor_t receptor);

/**
 * Signal the receptor and if there is another thread waiting
 * yield the CPU os it might run.
 *
 * @param receptor to signal
 */
//void ac_receptor_signal_yield_if_waiting(ac_receptor_t receptor);


#include <ac_receptor_impl.h>

#endif
