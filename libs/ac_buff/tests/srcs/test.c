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

#include <ac_buff.h>
#include <ac_buff_dbg.h>

#include <ac_inttypes.h>
#include <ac_memset.h>
#include <ac_memcmp.h>
#include <ac_test.h>

int main(void) {
  ac_bool error = AC_FALSE;

  // Test passing AC_NULL works
  AcBuff_print(AC_NULL, AC_NULL);

  ac_u32 count = 2;
  ac_u32 buff_size = 16;
  ac_u32 user_size = 8;

  AcBuff* buffs;
  /** test count == 0 is an error */
  error |= AC_TEST(AcBuff_alloc(AC_NULL, 0, 8, 8, &buffs) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(buffs == AC_NULL);

  /** test buff_size == 0 is an error */
  error |= AC_TEST(AcBuff_alloc(AC_NULL, count, 0, user_size, &buffs) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(buffs == AC_NULL);

  /** test buff_size < user_size is an error */
  error |= AC_TEST(
      AcBuff_alloc(AC_NULL, count, user_size - 1, user_size, &buffs) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(buffs == AC_NULL);

  // Success full test
  error |= AC_TEST(AcBuff_alloc(AC_NULL, count, buff_size, user_size, &buffs) == AC_STATUS_OK);
  error |= AC_TEST(buffs != AC_NULL);

  // Initialize data area
  for (ac_u32 i = 0; i < count; i++) {
    AcBuff* buff = AcBuff_get_nth(buffs, i, buff_size);
    ac_memset(buff->data, 0xa5, buff->hdr.buff_size);
    ac_memset(buff->data, i, buff->hdr.user_size);
  }

  // Test hdr and data contains expected values
  for (ac_u32 i = 0; i < count; i++) {
    AcBuff* buff = AcBuff_get_nth(buffs, i, buff_size);

    ac_u8 str[32];
    ac_sprintf(str, sizeof(str), "buffs[%d]=", i);
    AcBuff_print_count((char*)str, buff, buff->hdr.buff_size);

    error |= AC_TEST(buff->hdr.next == AC_NULL);
    error |= AC_TEST(buff->hdr.fifo == AC_NULL);
    error |= AC_TEST(buff->hdr.buff_size == buff_size);
    error |= AC_TEST(buff->hdr.user_size == user_size);

    ac_u32 j;
    for (j = 0; j < buff->hdr.user_size; j++) {
      error |= AC_TEST(buff->data[j] == i);
    }
    for (; j < buff->hdr.buff_size; j++) {
      error |= AC_TEST(buff->data[j] == 0xa5);
    }
  }

  AcBuff_free(buffs);

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
