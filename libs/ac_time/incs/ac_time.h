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

#ifndef SADIE_LIBS_AC_TIME_INCS_AC_TIME_H
#define SADIE_LIBS_AC_TIME_INCS_AC_TIME_H

#include <ac_inttypes.h>

#define AC_SEC_IN_NS 1000000000ll
#define AC_MIN_IN_NS (60ll * AC_SEC_IN_NS)
#define AC_HR_IN_NS (60ll * AC_MIN_IN_NS)
#define AC_DAY_IN_NS (24ll * AC_HR_IN_NS)
#define AC_YR_IN_NS (365ll * AC_DAY_IN_NS)

#define AC_SECS_PER_MIN (60ll)
#define AC_SECS_PER_HR (60ll * AC_SECS_PER_MIN)
#define AC_SECS_PER_DAY (24ll * AC_SECS_PER_HR)
#define AC_SECS_PER_YEAR (365ll * AC_SECS_PER_DAY)

#define LEADING_0 AC_TRUE
#define NO_LEADING_0 AC_FALSE

/**
 * Convert ticks to a duration string
 * y:day:min:sec.subsecs
 *
 * @param: ticks is value in ac_tsc_freq
 * @param: leading0 is true if leading zero values are to be printed
 * @param: precision number of digits of sub secs to print, <= 0 is none
 * @param: out_buff is output buffer
 * @param: out_buff_len is the length of the output buffer
 *
 * @return number of characters written to out_buf not including trailing 0
 *         if out_buff_len == 0 nothing is written
 *         if out_buff_len == 1 only a trailing zero is written
 */
ac_uint ac_ticks_to_duration_str(ac_u64 ticks, ac_bool leading0, ac_uint precision,
    ac_u8* out_buff, ac_uint out_buff_len);

/**
 * Initialize ac_time module.
 */
void ac_time_init(void);

#endif
