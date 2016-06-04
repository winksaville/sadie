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

#include <ac_inttypes.h>

/**
 * A AcReceptor type
 */
typedef struct AcReceptor AcReceptor;

/**
 * Get a receptor and set its state to NOT signaled
 *
 * @return AC_NULL if unable to get a receptor
 */
AcReceptor* AcReceptor_get(void);

/**
 * Return a receptor
 */
void AcReceptor_ret(AcReceptor* receptor);

/**
 * Wait for the receptor to be signaled only one entity can wait
 * on a receptor at a time. If the receptor has already been signaled
 * ac_receptor_wait will return immediately.
 *
 * @return 0 if successfull !0 if an error.
 */
ac_u32 AcReceptor_wait(AcReceptor* receptor);

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 */
void AcReceptor_signal(AcReceptor* receptor);

/**
 * Signal the receptor and if there is another thread waiting
 * yield the CPU os it might run.
 *
 * @param receptor to signal
 */
void AcReceptor_signal_yield_if_waiting(AcReceptor* receptor);

/**
 * Initialize this module early, must be
 * called before receptor_init
 */
void AcReceptor_early_init(void);

/**
 * Initialize this module defining the number of
 * sensors to support.
 */
void AcReceptor_init(ac_u32 max_receptors);

#endif
