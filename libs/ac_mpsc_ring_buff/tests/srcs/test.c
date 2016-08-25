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

#include <ac_mpsc_ring_buff.h>
#include <ac_mpsc_ring_buff_dbg.h>
#include <ac_mpsc_ring_buff_internal.h>

#include <ac_memmgr.h>
#include <ac_msg.h>
#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize AcMpscRingBuff *
 * return !0 if an error.
 */
AcBool test_init_and_deinit_mpsc_ring_buff() {
  AcBool error = AC_FALSE;
  AcMpscRingBuff rb;

  ac_printf("test_init_and_deinit_mpsc_ring_buff:+rb=%p\n", &rb);

  // Initialize
  ac_printf("test_init_deinit_mpsc_ring_buff: invoke init rb=%p\n", &rb);
  error |= AC_TEST(AcMpscRingBuff_init(&rb, 2) == AC_STATUS_OK);
  AcMpscRingBuff_print("test_init_deinit_mpsc_ring_buff: initialized rb:", &rb);

  error |= AC_TEST(rb.add_idx == 0);
  error |= AC_TEST(rb.rmv_idx == 0);
  error |= AC_TEST(rb.count == 0);
  error |= AC_TEST(rb.processed == 0);
  error |= AC_TEST(rb.ring_buffer != AC_NULL);

  // Deinitialize
  AcMpscRingBuff_print("test_init_deinit_mpsc_ring_buff: invoke deinit rb:", &rb);
  AcMpscRingBuff_deinit(&rb);
  ac_printf("test_init_deinit_mpsc_ring_buff: deinitialized rb=%p\n", &rb);

  error |= AC_TEST(rb.add_idx == 0);
  error |= AC_TEST(rb.rmv_idx == 0);
  error |= AC_TEST(rb.count == 0);
  error |= AC_TEST(rb.processed == 0);

  ac_printf("test_init_and_deinit_mpsc_ring_buff:-error=%d\n", error);

  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
AcBool test_add_rmv() {
  AcBool error = AC_FALSE;
  AcMpscRingBuff rb;
  AcU8* mem;
  const AcU32 data_size = 2;

  ac_printf("test_add_rmv:+rb=%p\n", &rb);

  // Initialize
  AcMpscRingBuff_init(&rb, 2);
  AcMpscRingBuff_print("test_add_rmv_mem rb:", &rb);

  // Add mem1
  AcU8* mems = ac_malloc(3 * data_size);
  error |= AC_TEST(mems != AC_NULL);
  mems[0] = 1;
  mems[1] = 2;
  mems[2] = 3;

  // Add first mem
  AcBool rslt = AcMpscRingBuff_add_mem(&rb, &mems[0]);
  error |= AC_TEST(rslt);
  AcMpscRingBuff_print("test_add_rmv: after add mems[0] rb:\n", &rb);
  error |= AC_TEST(rb.add_idx == 1);
  error |= AC_TEST(rb.rmv_idx == 0);

  // Add Second mem
  rslt = AcMpscRingBuff_add_mem(&rb, &mems[1]);
  error |= AC_TEST(rslt);
  AcMpscRingBuff_print("test_add_rmv_mem: after add mems[1] rb:", &rb);
  error |= AC_TEST(rb.add_idx == 2);
  error |= AC_TEST(rb.rmv_idx == 0);

  // Add third mem, should fail
  rslt = AcMpscRingBuff_add_mem(&rb, &mems[2]);
  error |= AC_TEST(rslt == AC_FALSE);
  AcMpscRingBuff_print("test_add_rmv_mem: after add mems[2] rb same as add mems[1]:", &rb);
  error |= AC_TEST(rb.add_idx == 2);
  error |= AC_TEST(rb.rmv_idx == 0);

  // Remove first mem
  mem = AcMpscRingBuff_rmv_mem(&rb);
  AcMpscRingBuff_print("test_add_rmv_mem: after first rmv:", &rb);
  ac_printf("test_add_rmv_mem: mem=%p mem[0]=%d\n", mem, mem[0]);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem[0] == 1);
  error |= AC_TEST(rb.add_idx == 2);
  error |= AC_TEST(rb.rmv_idx == 1);

  // Remove second mem
  mem = AcMpscRingBuff_rmv_mem(&rb);
  AcMpscRingBuff_print("test_add_rmv_mem: after first rmv:", &rb);
  ac_printf("test_add_rmv_mem: mem=%p mem[0]=%d\n", mem, mem[0]);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem[0] == 2);
  error |= AC_TEST(rb.add_idx == 2);
  error |= AC_TEST(rb.rmv_idx == 2);

  // Remove from empty which should be null
  mem = AcMpscRingBuff_rmv_mem(&rb);
  error |= AC_TEST(mem == AC_NULL);

  // Add third mem
  rslt = AcMpscRingBuff_add_mem(&rb, &mems[2]);
  error |= AC_TEST(rslt);
  AcMpscRingBuff_print("test_add_rmv_mem: after add mems[2] rb:", &rb);
  error |= AC_TEST(rb.add_idx == 3);
  error |= AC_TEST(rb.rmv_idx == 2);

  // Remove third mem
  mem = AcMpscRingBuff_rmv_mem(&rb);
  AcMpscRingBuff_print("test_add_rmv_mem: after rmv:", &rb);
  ac_printf("test_add_rmv_mem: mem=%p mem[0]=%d\n", mem, mem[0]);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem[0] == 3);
  error |= AC_TEST(rb.add_idx == 3);
  error |= AC_TEST(rb.rmv_idx == 3);

  // Remove from empty which should be null
  mem = AcMpscRingBuff_rmv_mem(&rb);
  error |= AC_TEST(mem == AC_NULL);

  // Deinitialize
  AcMpscRingBuff_deinit(&rb);

  // Free mems
  ac_free(mems);

  ac_printf("test_add_rmv_mem:-error=%d\n", error);
  return error;
}

int main(void) {
  AcBool error = AC_FALSE;

  error |= test_init_and_deinit_mpsc_ring_buff();
  ac_printf("\n");
  error |= test_add_rmv();
  ac_printf("\n");

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
