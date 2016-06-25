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

#include <ac_mem.h>
#include <ac_mem_dbg.h>

#include <ac_inttypes.h>
#include <ac_memset.h>
#include <ac_memcmp.h>
#include <ac_test.h>

int main(void) {
  ac_bool error = AC_FALSE;

  // Test passing AC_NULL works
  AcMem_print(AC_NULL, AC_NULL);

  ac_u32 count = 2;
  ac_u32 data_size = 16;
  ac_u32 user_size = 8;

  AcMem* mems;
  /** test count == 0 is an error */
  error |= AC_TEST(AcMem_alloc(AC_NULL, 0, 8, 8, &mems) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mems == AC_NULL);

  /** test data_size == 0 is an error */
  error |= AC_TEST(AcMem_alloc(AC_NULL, count, 0, user_size, &mems) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mems == AC_NULL);

  /** test data_size < user_size is an error */
  error |= AC_TEST(
      AcMem_alloc(AC_NULL, count, user_size - 1, user_size, &mems) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mems == AC_NULL);

  // Success full test
  error |= AC_TEST(AcMem_alloc(AC_NULL, count, data_size, user_size, &mems) == AC_STATUS_OK);
  error |= AC_TEST(mems != AC_NULL);

  // Initialize data area
  for (ac_u32 i = 0; i < count; i++) {
    AcMem* mem = AcMem_get_nth(mems, i);
    ac_memset(mem->data, 0xa5, mem->hdr.data_size);
    ac_memset(mem->data, i, mem->hdr.user_size);
  }

  // Test hdr and data contains expected values
  for (ac_u32 i = 0; i < count; i++) {
    AcMem* mem = AcMem_get_nth(mems, i);

    char str[32];
    ac_sprintf((ac_u8*)str, sizeof(str), "mems[%d]=", i);
    AcMem_print_count(str, mem, mem->hdr.data_size);

    error |= AC_TEST(mem->hdr.next == AC_NULL);
    error |= AC_TEST(mem->hdr.pool_fifo == AC_NULL);
    error |= AC_TEST(mem->hdr.data_size == data_size);
    error |= AC_TEST(mem->hdr.user_size == user_size);

    ac_u32 j;
    for (j = 0; j < mem->hdr.user_size; j++) {
      error |= AC_TEST(mem->data[j] == i);
    }
    for (; j < mem->hdr.data_size; j++) {
      error |= AC_TEST(mem->data[j] == 0xa5);
    }
  }

  AcMem_free(mems);

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
