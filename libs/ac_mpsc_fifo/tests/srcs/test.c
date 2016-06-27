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

#include <ac_mpsc_fifo.h>
#include <ac_mpsc_fifo_dbg.h>

#include <ac_mem.h>
#include <ac_mem_dbg.h>
#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize AcMpscFifo *
 * return !0 if an error.
 */
static ac_bool test_init_and_deinit_mpscfifo() {
  ac_bool error = AC_FALSE;
  AcMpscFifo fifo;

  ac_printf("test_init_and_deinit:+fifo=%p\n", &fifo);

  // Initialize
  ac_printf("test_init_deinit: invoke init fifo=%p\n", &fifo);
  error |= AC_TEST(AcMpscFifo_init(&fifo, 1) == AC_STATUS_OK);
  AcMpscFifo_print("test_init_deinit: initialized fifo:", &fifo);

  error |= AC_TEST(fifo.head != AC_NULL);
  error |= AC_TEST(fifo.tail != AC_NULL);
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);

  // Deinitialize
  AcMpscFifo_print("test_init_deinit: invoke deinit fifo:", &fifo);
  AcMpscFifo_deinit(&fifo);
  ac_printf("test_init_deinit: deinitialized fifo=%p\n", &fifo);

  error |= AC_TEST(fifo.head == AC_NULL);
  error |= AC_TEST(fifo.tail == AC_NULL);

  ac_printf("test_init_and_deinit:-error=%d\n", error);

  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
ac_bool test_add_rmv_ac_mem() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  AcMpscFifo fifo;
  AcMem* mem;
  ac_u32 data_size = 2;

  ac_printf("test_add_rmv_ac_mem:+fifo=%p\n", &fifo);

  // Initialize
  AcMpscFifo_init(&fifo, data_size);
  AcMpscFifo_print("test_add_rmv_ac_mem fifo:", &fifo);

  // Add mem1
  AcMem* mems;
  status = AcMem_alloc(AC_NULL, 2, data_size, data_size, &mems);
  error |= AC_TEST(status == 0);
  AcMem_get_nth(mems, 0)->data[0] = 1;
  AcMem_get_nth(mems, 0)->data[1] = 2;
  AcMem_print("test_add_rmv_ac_mem: mems[0]=", AcMem_get_nth(mems, 0));
  AcMem_get_nth(mems, 1)->data[0] = 3;
  AcMem_get_nth(mems, 1)->data[1] = 4;
  AcMem_print("test_add_rmv_ac_mem: mems[1]=", AcMem_get_nth(mems, 1));

  // Add first mem
  AcMpscFifo_add_ac_mem(&fifo, AcMem_get_nth(mems, 0));
  AcMpscFifo_print("test_add_rmv_ac_mem: after add mems[0] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 0));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // Add Second mem
  AcMpscFifo_add_ac_mem(&fifo, AcMem_get_nth(mems, 1));
  AcMpscFifo_print("test_add_rmv_ac_mem: after add mems[1] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 1));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // Remove first mem
  mem = AcMpscFifo_rmv_ac_mem(&fifo);
  AcMem_print("test_add_rmv_ac_mem: after rmv mems=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.next == AC_NULL);
  error |= AC_TEST(mem->data[0] == 1);
  error |= AC_TEST(mem->data[1] == 2);
  AcMpscFifo_print("test_add_rmv_ac_mem: after rmv mems[0] fifo:", &fifo);

  // Remove second mem
  mem = AcMpscFifo_rmv_ac_mem(&fifo);
  AcMem_print("test_add_rmv_ac_mem: after rmv mems=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.next == AC_NULL);
  error |= AC_TEST(mem->data[0] == 3);
  error |= AC_TEST(mem->data[1] == 4);
  AcMpscFifo_print("test_add_rmv_ac_mem: after rmv mems[1] fifo:", &fifo);

  // Remove from empty which should be null
  mem = AcMpscFifo_rmv_ac_mem(&fifo);
  error |= AC_TEST(mem == AC_NULL);

  // Deinitialize
  AcMpscFifo_deinit(&fifo);

  // Free mems
  AcMem_free(mems);

  ac_printf("test_add_rmv_ac_mem:-error=%d\n", error);
  return error;
}

/**
 * Test we can add and remove raw msgs from a FIFO.
 *
 * return !0 if an error.
 */

ac_bool test_add_rmv_ac_mem_raw() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  AcMpscFifo fifo;
  AcMem* mem;
  ac_u32 data_size = 2;

  ac_printf("test_add_rmv_ac_mem_raw:+fifo=%p\n", &fifo);

  // Initialize
  AcMpscFifo_init(&fifo, data_size);
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: fifo:", &fifo);

  // Allocate 3 memers
  AcMem* mems;
  status = AcMem_alloc(AC_NULL, 3, data_size, data_size, &mems);
  error |= AC_TEST(status == 0);
  AcMem_get_nth(mems, 0)->data[0] = 1;
  AcMem_get_nth(mems, 0)->data[1] = 2;
  AcMem_print("test_add_rmv_ac_mem_raw: mems[0]=", AcMem_get_nth(mems, 0));
  AcMem_get_nth(mems, 1)->data[0] = 3;
  AcMem_get_nth(mems, 1)->data[1] = 4;
  AcMem_print("test_add_rmv_ac_mem_raw: mems[1]=", AcMem_get_nth(mems, 1));
  AcMem_get_nth(mems, 2)->data[0] = 5;
  AcMem_get_nth(mems, 2)->data[1] = 6;
  AcMem_print("test_add_rmv_ac_mem_raw: mems[2]=", AcMem_get_nth(mems, 2));

  // Add first mem
  AcMpscFifo_add_ac_mem(&fifo, AcMem_get_nth(mems, 0));
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: after add mems[0] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 0));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // Add second mem
  AcMpscFifo_add_ac_mem(&fifo, AcMem_get_nth(mems, 1));
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: after add mems[1] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 1));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // Add third mem
  AcMpscFifo_add_ac_mem(&fifo, AcMem_get_nth(mems, 2));
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: after add mems[2] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcMem_get_nth(mems, 2));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcMem_get_nth(mems, 0));

  // First remove removes the stub since this is raw and the contents must be 0
  mem = AcMpscFifo_rmv_ac_mem_raw(&fifo);
  AcMem_print("test_add_rmv_ac_mem_raw: first rmv mem=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.next == AC_NULL);
  error |= AC_TEST(mem->hdr.user_size == 0);
  error |= AC_TEST(mem->data[0] == 0);
  error |= AC_TEST(mem->data[1] == 0);
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: first rmv fifo:", &fifo);

  // Second remove removes the first mem, again because this is raw
  mem = AcMpscFifo_rmv_ac_mem_raw(&fifo);
  AcMem_print("test_add_rmv_ac_mem_raw: second rmv mem=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.next == AC_NULL);
  error |= AC_TEST(mem->hdr.user_size == 2);
  error |= AC_TEST(mem->data[0] == 1);
  error |= AC_TEST(mem->data[1] == 2);
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: second rmv fifo:", &fifo);

  // Third remove removes the second mem, again because this is raw
  mem = AcMpscFifo_rmv_ac_mem_raw(&fifo);
  AcMem_print("test_add_rmv_ac_mem_raw: third rmv mem=", mem);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.next == AC_NULL);
  error |= AC_TEST(mem->hdr.user_size == 2);
  error |= AC_TEST(mem->data[0] == 3);
  error |= AC_TEST(mem->data[1] == 4);
  AcMpscFifo_print("test_add_rmv_ac_mem_raw: third rmv fifo:", &fifo);

  // Remove from empty which should be null
  mem = AcMpscFifo_rmv_ac_mem(&fifo);
  error |= AC_TEST(mem == AC_NULL);

  // Deinitialize
  AcMpscFifo_deinit(&fifo);

  // Free mems
  AcMem_free(mems);

  ac_printf("test_add_rmv_ac_mem_raw:-error=%d\n", error);
  return error;
}

/**
 * Test we can add and remove multiple AcMem's owned by the fifo
 *
 * return !0 if an error.
 */

ac_bool test_init_and_alloc_multiple() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  AcMpscFifo fifo;
  ac_u32 count = 3;
  ac_u32 data_size = 2;

  ac_printf("test_init_and_alloc_multiple:+fifo=%p\n", &fifo);

  // Initialize
  status = AcMpscFifo_init_and_alloc(&fifo, count, data_size);
  ac_printf("test_init_and_alloc_multiple: init_alloc fifo=%p status=%d\n", &fifo, status);
  if (status != AC_STATUS_OK) {
    error |= AC_TRUE;
    goto done;
  }
  AcMpscFifo_print("test_init_and_alloc_multiple: after init_alloc fifo:", &fifo);

  // Loop through validating the initial conditions and initializing data
  for (ac_u32 i = 0; i < count; i++) {
    // Remove next AcMem and validate
    AcMem* mem = AcMpscFifo_rmv_ac_mem(&fifo);
    error |= AC_TEST(mem != AC_NULL);
    error |= AC_TEST(mem->hdr.next == AC_NULL);
    error |= AC_TEST(mem->hdr.data_size == data_size);
    error |= AC_TEST(mem->hdr.user_size == 0);
    error |= AC_TEST(mem->data[0] == 0);
    error |= AC_TEST(mem->data[1] == 0);
    error |= AC_TEST(data_size >= 2);

    // Initialize data fields
    mem->hdr.user_size = data_size;
    mem->data[0] = i;
    mem->data[1] = i + 1;

    // Add it back
    AcMpscFifo_add_ac_mem(&fifo, mem);
  }
  AcMpscFifo_print("test_init_and_alloc_multiple: after initialization fifo:", &fifo);

  // Loop through validating the initialized data
  for (ac_u32 i = 0; i < count; i++) {
    // Remove the first one
    AcMem* mem = AcMpscFifo_rmv_ac_mem(&fifo);

    // Validate field
    error |= AC_TEST(mem != AC_NULL);
    error |= AC_TEST(mem->hdr.next == AC_NULL);
    error |= AC_TEST(mem->hdr.data_size == data_size);
    error |= AC_TEST(mem->hdr.user_size == data_size);
    error |= AC_TEST(mem->data[0] == i);
    error |= AC_TEST(mem->data[1] == i + 1);

    // Put it back
    AcMpscFifo_add_ac_mem(&fifo, mem);
  }
  AcMpscFifo_print("test_init_and_alloc_multiple: after validating fifo:", &fifo);

  // Deinitialize
  AcMpscFifo_deinit_full(&fifo);

done:
  ac_printf("test_init_and_alloc_multiple:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_init_and_deinit_mpscfifo();
  ac_printf("\n");
  error |= test_add_rmv_ac_mem();
  ac_printf("\n");
  error |= test_add_rmv_ac_mem_raw();
  ac_printf("\n");
  error |= test_init_and_alloc_multiple();
  ac_printf("\n");

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
