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

#include <ac_mpsc_fifo.h>
#include <ac_mpsc_fifo_dbg.h>

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_memmgr.h>
#include <ac_memset.h>
#include <ac_mem.h>
#include <ac_mpsc_fifo_dbg.h>
#include <ac_receptor.h>
#include <ac_status.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>
#include <ac_thread.h>

ac_bool simple_mpsc_fifo_perf(ac_u64 loops) {
  AcStatus status;
  ac_bool error = AC_FALSE;
  ac_debug_printf("simple_mpsc_fifo_perf:+loops=%lu\n", loops);

  AcMpscFifo fifo;
  AcMem* mem;

  ac_u32 count = 2;
  ac_u32 data_size = 2;

  status = AcMpscFifo_init_and_alloc(&fifo, count, data_size);
  ac_printf("simple_mpsc_fifo_perf: init_alloc fifo=%p status=%d\n", &fifo, status);
  if (status != AC_STATUS_OK) {
    error |= AC_TRUE;
    goto done;
  }

  AcMem** mems = ac_malloc(count * sizeof(AcMem*));
  ac_printf("simple_mpsc_fifo_perf: get an array of mems=%p\n", mems);
  if (mems == AC_NULL) {
    error |= AC_TRUE;
    goto done;
  }

  // Empty the fifo.
  for (ac_u32 i = 0; i < count; i++) {
    mems[i] = AcMpscFifo_rmv_ac_mem(&fifo);
    ac_debug_printf("mems[%d]=%p\n", i, mems[i]);
  }

  ac_u64 start = ac_tscrd();

  // Be careful, since the address of an AcMem changes
  // we need to add a "new" one back each time.
  mem = mems[0];
  for (ac_u64 i = 0; i < loops; i++) {
    AcMpscFifo_add_ac_mem(&fifo, mem);
    mem = AcMpscFifo_rmv_ac_mem(&fifo);
    ac_debug_printf("%d mem=%p\n", i, mem);
  }

  ac_u64 stop = ac_tscrd();

  ac_u64 duration = stop - start;
  ac_u64 ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_fifo_perf: empty     duration=%lu time=%.9t ns_per_op=%.3Sns\n",
      duration, duration, ns_per_op);


  // Testing adding a removing a single AcMem to a non-empty fifo
  AcMpscFifo_add_ac_mem(&fifo, mems[0]);
  start = ac_tscrd();

  // As before, be careful
  mem = mems[1];
  for (ac_u64 i = 0; i < loops; i++) {
    AcMpscFifo_add_ac_mem(&fifo, mem);
    mem = AcMpscFifo_rmv_ac_mem(&fifo);
    ac_debug_printf("%d mem=%p\n", i, mem);
  }

  stop = ac_tscrd();

  duration = stop - start;
  ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_fifo_perf: non-empty duration=%lu time=%.9t ns_per_op=%.3Sns\n",
      duration, duration, ns_per_op);

  // Add the last one back so the fifo is "full"
  AcMpscFifo_add_ac_mem(&fifo, mem);

  AcMpscFifo_deinit_full(&fifo);

 done:

  ac_printf("simple_mpsc_fifo_perf:-\n");
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

  error |= simple_mpsc_fifo_perf(200000000);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
