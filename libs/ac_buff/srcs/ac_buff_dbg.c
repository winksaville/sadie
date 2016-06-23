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

/**
 * Debug code for AcBuff
 */

#include <ac_buff_dbg.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

/**
 * Print a AcBuff and the specified number of bytes in data
 */
void AcBuff_print_count(const char* leader, AcBuff* buff, ac_u32 data_count_to_print) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  if (buff != AC_NULL) {
    ac_printf("%p next=%p pool_fifo=%p data_size=%d user_size=%d data=",
        (void *)buff, (void *)(buff->hdr.next), buff->hdr.pool_fifo,
        buff->hdr.data_size, buff->hdr.user_size);
    for (ac_u32 i = 0; i < data_count_to_print; i++) {
      if (i == 0) {
        ac_printf("%2x", buff->data[i]);
      } else {
        ac_printf(" %2x", buff->data[i]);
      }
    }
    ac_printf("\n");
  } else {
    ac_printf("buff == AC_NULL\n");
  }
}

/**
 * Print a AcBuff and all of the user data
 */
void AcBuff_print(const char* leader, AcBuff* buff) {
  if (buff != AC_NULL) {
    AcBuff_print_count(leader, buff, buff->hdr.user_size);
  } else {
    ac_printf("buff == AC_NULL\n");
  }
}
