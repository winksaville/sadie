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

#include <ac_mpsc_fifo_in.h>
#include <ac_mpsc_fifo_in_dbg.h>
#include <ac_mpsc_fifo_in_internal.h>

#include <ac_memmgr.h>
#include <ac_mem.h>
#include <ac_mem_dbg.h>
#include <ac_msg.h>
#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize AcMpscFifoIn *
 * return !0 if an error.
 */
ac_bool test_init_and_deinit_mpsc_fifo_in() {
  ac_bool error = AC_FALSE;
  AcMpscFifoIn fifo;

  ac_printf("test_init_and_deinit_fifo_in:+fifo=%p\n", &fifo);

  // Initialize
  ac_printf("test_init_deinit_fifo_in: invoke init fifo=%p\n", &fifo);
  error |= AC_TEST(AcMpscFifoIn_init(&fifo) == AC_STATUS_OK);
  AcMpscFifoIn_print("test_init_deinit_fifo_in: initialized fifo:", &fifo);

  error |= AC_TEST(fifo.head != AC_NULL);
  error |= AC_TEST(fifo.tail != AC_NULL);
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);

  // Deinitialize
  AcMpscFifoIn_print("test_init_deinit_fifo_in: invoke deinit fifo:", &fifo);
  AcMpscFifoIn_deinit(&fifo);
  ac_printf("test_init_deinit_fifo_in: deinitialized fifo=%p\n", &fifo);

  error |= AC_TEST(fifo.head == AC_NULL);
  error |= AC_TEST(fifo.tail == AC_NULL);

  ac_printf("test_init_and_deinit_fifo_in:-error=%d\n", error);

  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
ac_bool test_add_rmv_mpsc_fifo_in() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  AcMpscFifoIn fifo;
  AcMem* mem;
  const ac_u32 data_size = 2;

  ac_printf("test_add_rmv_ac_mem_mpsc_fifo_in:+fifo=%p\n", &fifo);

  // Initialize
  AcMpscFifoIn_init(&fifo);
  AcMpscFifoIn_print("test_add_rmv_ac_mem_mpsc_fifo_in fifo:", &fifo);

  // Add mem1
  AcMem* mems;
  status = AcMem_alloc(AC_NULL, 2, data_size, data_size, &mems);
  error |= AC_TEST(status == 0);
  AcMem_get_nth(mems, 0)->data[0] = 1;
  AcMem_get_nth(mems, 0)->data[1] = 2;
  AcMem_print("test_add_rmv_ac_mem_mpsc_fifo_in: mems[0]=", AcMem_get_nth(mems, 0));
  AcMem_get_nth(mems, 1)->data[0] = 3;
  AcMem_get_nth(mems, 1)->data[1] = 4;
  AcMem_print("test_add_rmv_ac_mem_mpsc_fifo_in: mems[1]=", AcMem_get_nth(mems, 1));

  // Add first mem
  AcMpscFifoIn_add_ac_mem(&fifo, AcMem_get_nth(mems, 0));
  AcMpscFifoIn_print("test_add_rmv_ac_mem_mpsc_fifo_in: after add mems[0] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 0));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // Add Second mem
  AcMpscFifoIn_add_ac_mem(&fifo, AcMem_get_nth(mems, 1));
  AcMpscFifoIn_print("test_add_rmv_ac_mem_mpsc_fifo_in: after add mems[1] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 1));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // Remove first mem
  mem = AcMpscFifoIn_rmv_ac_mem(&fifo);
  AcMem_print("test_add_rmv_ac_mem_mpsc_fifo_in: after rmv mems=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->data[0] == 1);
  error |= AC_TEST(mem->data[1] == 2);
  AcMpscFifoIn_print("test_add_rmv_ac_mem_mpsc_fifo_in: after rmv mems[0] fifo:", &fifo);

  // Remove second mem
  mem = AcMpscFifoIn_rmv_ac_mem(&fifo);
  AcMem_print("test_add_rmv_ac_mem_mpsc_fifo_in: after rmv mems=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->data[0] == 3);
  error |= AC_TEST(mem->data[1] == 4);
  AcMpscFifoIn_print("test_add_rmv_ac_mem_mpsc_fifo_in: after rmv mems[1] fifo:", &fifo);

  // Remove from empty which should be null
  mem = AcMpscFifoIn_rmv_ac_mem(&fifo);
  error |= AC_TEST(mem == AC_NULL);

  // Deinitialize
  AcMpscFifoIn_deinit(&fifo);

  // Free mems
  AcMem_free(mems);

  ac_printf("test_add_rmv_ac_mem_mpsc_fifo_in:-error=%d\n", error);
  return error;

}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_init_and_deinit_mpsc_fifo_in();
  ac_printf("\n");
  error |= test_add_rmv_mpsc_fifo_in();
  ac_printf("\n");

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
