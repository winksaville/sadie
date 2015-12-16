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

#ifndef SADIE_ARCH_ARMV6_ARM1176JZF_S_LIBS_AC_THREAD_SRCS_TIMER_H
#define SADIE_ARCH_ARMV6_ARM1176JZF_S_LIBS_AC_THREAD_SRCS_TIMER_H

#include <ac_inttypes.h>

/**
 * Start periodic timer.
 *
 * @param period_in_micro_secs
 */
void ac_start_periodic_timer(ac_u32 period_in_micro_secs);

#endif
