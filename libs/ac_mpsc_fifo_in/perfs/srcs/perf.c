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

#define NDEBUG

#include <ac_mpsc_fifo_in.h>
#include <ac_mpsc_fifo_in_dbg.h>
#include <ac_mpsc_fifo_in_internal.h>

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_memset.h>
#include <ac_mem.h>
#include <ac_mpsc_fifo_dbg.h>
#include <ac_receptor.h>
#include <ac_status.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>
#include <ac_thread.h>

ac_bool simple_mpsc_fifo_in_perf(ac_u64 loops) {
  AcStatus status;
  ac_bool error = AC_FALSE;
  ac_debug_printf("simple_mpsc_fifo_in_perf:+loops=%lu\n", loops);

  AcMpscFifoIn fifo;

  const ac_u32 data_size = 2;
  AcMem* mems;
  status = AcMem_alloc(AC_NULL, 2, data_size, data_size, &mems);
  if (status != AC_STATUS_OK) {
    ac_printf("simple_mpsc_fifo_in_perf:-AcMem_alloc returned status=%d\n", status);
    return AC_FALSE;
  }
  AcMem_get_nth(mems, 0)->data[0] = 1;
  AcMem_get_nth(mems, 0)->data[1] = 2;
  AcMem_get_nth(mems, 1)->data[0] = 3;
  AcMem_get_nth(mems, 1)->data[1] = 4;

  // Testing adding a removing a single AcMem to an empty fifo
  error |= AC_TEST(AcMpscFifoIn_init(&fifo) == AC_STATUS_OK);

  ac_u64 start = ac_tscrd();

  for (ac_u64 i = 0; i < loops; i++) {
    AcMpscFifoIn_add_ac_mem(&fifo, AcMem_get_nth(mems, 0));
    AcMpscFifoIn_rmv_ac_mem(&fifo);
  }

  ac_u64 stop = ac_tscrd();

  ac_u64 duration = stop - start;
  ac_u64 ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_fifo_in_perf: empty     duration=%lu time=%.9t ns_per_op=%.3Sns\n",
      duration, duration, ns_per_op);


  // Testing adding a removing a single AcMem to a non-empty fifo
  AcMpscFifoIn_add_ac_mem(&fifo, AcMem_get_nth(mems, 0));
  start = ac_tscrd();

  for (ac_u64 i = 0; i < loops; i++) {
    AcMpscFifoIn_add_ac_mem(&fifo, AcMem_get_nth(mems, 1));
    AcMpscFifoIn_rmv_ac_mem(&fifo);
  }

  stop = ac_tscrd();

  duration = stop - start;
  ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_fifo_in_perf: non-empty duration=%lu time=%.9t ns_per_op=%.3Sns\n",
      duration, duration, ns_per_op);


  AcMpscFifoIn_deinit(&fifo);

  ac_printf("simple_mpsc_fifo_in_perf:-\n");
  return error;
}

/**
 * main
 */
int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  AcReceptor_init(50);
  AcTime_init();

  ac_debug_printf("sizeof(AcMem)=%d\n", sizeof(AcMem));

  error |= simple_mpsc_fifo_in_perf(200000000);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
