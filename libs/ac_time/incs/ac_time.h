/* * Copyright 2015 Wink Saville
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
 * Convert nano seconds to ticks
 *
 * @param: nanos
 *
 * @return: number of ticks
 */
ac_u64 AcTime_nanos_to_ticks(ac_u64 nanos);

/**
 * Convert ticks to nano seconds
 *
 * @param: ticks
 *
 * @return: number of nano seconds
 */
ac_u64 AcTime_ticks_to_nanos(ac_u64 ticks);

/**
 * Convert ticks to a duration string of the form XyXdXhXmX.Ys
 * So 3 years 5 days 1 hour 7 minutes 3.201 seconds is
 * printed as 3y5d1h7m3.201s
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
ac_uint AcTime_ticks_to_duration_str(ac_u64 ticks, ac_bool leading0, ac_uint precision,
    ac_u8* out_buff, ac_uint out_buff_len);

/**
 * Convert ticks to 'seconds' float string as whole.fraction
 *
 * @param: ticks is value in ac_tsc_freq
 * @param: leading_0 is true if leading zero values are to be printed
 * @param: precision is number of digits after seconds decimal place
 * @param: units_str appended to end
 * @param: out_buf is output buffer
 * @param: out_buf_len is the length of the output buffer
 *
 * @return number of characters written to out_buf not including trailing 0
 *         if out_buf_len == 0 nothing is written
 *         if out_buf_len == 1 only a trailing zero is written
 */
ac_uint AcTime_ticks_to_seconds_str(ac_u64 ticks, ac_bool leading_0, ac_uint precision,
    char* units_str, ac_u8* out_buff, ac_uint out_buff_len);

/**
 * Initialize ac_time early.
 */
void AcTime_early_init(void);

/**
 * Initialize ac_time module.
 */
void AcTime_init(void);

#endif
