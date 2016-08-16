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

#include <ac_mpsc_ring_buff.h>
#include <ac_mpsc_ring_buff_dbg.h>
#include <ac_mpsc_ring_buff_internal.h>

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_memset.h>
#include <ac_memmgr.h>
#include <ac_mpsc_fifo_dbg.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>
#include <ac_thread.h>

extern AcMpscFifo* AcMpscRingBuff_get_fifo(AcMpscRingBuff* pool);

ac_bool simple_mpsc_ring_buff_perf(ac_u64 loops) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("simple_mpsc_ring_buff_perf:+ loops=%lu\n", loops);

  AcMpscRingBuff rb;

  ac_u8* mems = ac_malloc(2 * sizeof(ac_u8));

  // Testing a pool one entry
  error |= AC_TEST(AcMpscRingBuff_init(&rb, 2) == AC_STATUS_OK);

  // Test add remove with an empty ring buffer
  ac_u64 start = ac_tscrd();

  for (ac_u64 i = 0; i < loops; i++) {
    AcMpscRingBuff_add_mem(&rb, &mems[0]);
    AcMpscRingBuff_rmv_mem(&rb);
  }

  ac_u64 stop = ac_tscrd();

  ac_u64 duration = stop - start;
  ac_u64 ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_ring_buff_perf: empty     duration=%lu time=%.9t ns_per_op=%.4S\n",
      duration, duration, ns_per_op);

  // Test add remove with an empty ring buffer
  AcMpscRingBuff_add_mem(&rb, &mems[0]);
  start = ac_tscrd();

  for (ac_u64 i = 0; i < loops; i++) {
    AcMpscRingBuff_add_mem(&rb, &mems[1]);
    AcMpscRingBuff_rmv_mem(&rb);
  }

  stop = ac_tscrd();

  duration = stop - start;
  ns_per_op = (duration * AC_SEC_IN_NS) / loops;
  ac_printf("simple_mpsc_ring_buff_perf: non-empty duration=%lu time=%.9t ns_per_op=%.4S\n",
      duration, duration, ns_per_op);

  AcMpscRingBuff_deinit(&rb);

  ac_printf("simple_mpsc_ring_buff_perf:-\n");
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

  error |= simple_mpsc_ring_buff_perf(200000000);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
