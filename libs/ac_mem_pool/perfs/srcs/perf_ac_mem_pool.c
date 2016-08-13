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

#include <ac_mem_pool.h>
#include <ac_mem_pool_dbg.h>
#include <ac_mem_pool_internal.h>
#include <ac_mem_pool/perfs/incs/perf.h>

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_memset.h>
#include <ac_mpsc_fifo_dbg.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

extern AcMpscFifo* AcMemPool_get_fifo(AcMemPool* pool);

ac_bool simple_mem_pool_perf() {
  ac_bool error = AC_FALSE;
  ac_debug_printf("simple_mem_pool_perf:+\n");

  AcMemPool* mp;
  AcMem* mem;
  AcMem* mem2;

  AcMemPoolCountSize mpcs[1] = {
    { .count = 1, .data_size = 1 },
  };

  // Test BAD_PARAMS
  ac_debug_printf("simple_mem_pool_perf: create a pool with NO AcMem's\n");
  error |= AC_TEST(AcMemPool_alloc(0, mpcs, &mp) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mp == AC_NULL);
  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mpcs), AC_NULL, &mp) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mp == AC_NULL);
  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mpcs), mpcs, AC_NULL) == AC_STATUS_BAD_PARAM);
  ac_debug_printf("\n");

  // Test requesting an AcMem from a AC_NULL pool returns AC_NULL
  ac_debug_printf("simple_mem_pool_perf: bad params for AcMem_get_ac_mem\n");
  error |= AC_TEST(AcMemPool_get_ac_mem(AC_NULL, 1) == AC_NULL);
  error |= AC_TEST(AcMemPool_get_ac_mem(AC_NULL, 0) == AC_NULL);
  ac_debug_printf("\n");

  ac_debug_printf("simple_mem_pool_perf: bad params for AcMem_get_mem\n");
  error |= AC_TEST(AcMemPool_get_mem(AC_NULL, 1) == AC_NULL);
  error |= AC_TEST(AcMemPool_get_mem(AC_NULL, 0) == AC_NULL);
  ac_debug_printf("\n");

  ac_debug_printf("simple_mem_pool_perf: AC_NULL passed to AcMem_free, AcMem_ret_ac_mem and AcMem_ret_mem\n");
  AcMemPool_free(AC_NULL);
  AcMemPool_ret_ac_mem(AC_NULL);
  AcMemPool_ret_mem(AC_NULL);
  ac_debug_printf("\n");

  // Testing creating a pool with single size
  ac_debug_printf("simple_mem_pool_perf: pool with one size data_size=%d\n",
      mpcs[0].data_size);
  error |= AC_TEST(AcMemPool_alloc(1, mpcs, &mp) == AC_STATUS_OK);
  error |= AC_TEST(mp != AC_NULL);
  AcMemPool_debug_print("simple_mem_pool_perf: pool after creation, should have one entry:", mp);
  ac_debug_printf("\n");

  // Test getting an AcMem
  ac_debug_printf("simple_mem_pool_perf: first get ac_mem, expecting != AC_NULL\n");
  mem = AcMemPool_get_ac_mem(mp, 1);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.user_size == 1);
  error |= AC_TEST(mem->data[0] == 0);
  AcMemPool_debug_print("simple_mem_pool_perf: pool after first get, should be empty:", mp);
  ac_debug_printf("\n");

  // Test a second get fails
  ac_debug_printf("simple_mem_pool_perf: second get ac_mem, expecting == AC_NULL\n");
  mem2 = AcMemPool_get_ac_mem(mp, 1);
  error |= AC_TEST(mem2 == AC_NULL);
  AcMemPool_debug_print("simple_mem_pool_perf: pool after second get:", mp);
  ac_debug_printf("\n");

  // Modify data and return the AcMem
  ac_debug_printf("simple_mem_pool_perf: ret ac_mem, expecting == AC_NULL\n");
  mem->data[0] = 1;
  error |= AC_TEST(mem->data[0] == 1);
  AcMemPool_ret_ac_mem(mem);
  AcMemPool_debug_print("simple_mem_pool_perf: pool after return, should have one entry:", mp);
  ac_debug_printf("\n");

  // Request it back and verify that data[0] is 0 and its not the same as mem
  ac_debug_printf("simple_mem_pool_perf: re-getting ac_mem, expecting != AC_NULL\n");
  mem2 = AcMemPool_get_ac_mem(mp, 1);
  error |= AC_TEST(mem2 != AC_NULL);
  error |= AC_TEST(mem2 != mem);
  error |= AC_TEST(mem2->hdr.user_size == 1);
  error |= AC_TEST(mem2->data[0] == 0);
  AcMemPool_debug_print("simple_mem_pool_perf: pool after re-getting, should be empty:", mp);
  ac_debug_printf("\n");

  ac_debug_printf("simple_mem_pool_perf:-\n");
  return error;
}

ac_bool multiple_mem_pool_perf() {
  ac_bool error = AC_FALSE;
  ac_debug_printf("multiple_mem_pool_perf:+\n");

  AcMemPool* mp;
  AcMem* ac_mem[3];
  ac_u8* mem[6];

  AcMemPoolCountSize mpcs[3] = {
    { .count = 1, .data_size = 1 },
    { .count = 2, .data_size = 2 },
    { .count = 3, .data_size = 3 },
  };

  // Create a pool with multiple sizes and counts
  ac_debug_printf("multiple_mem_pool_perf: pool with multiple sizes data_sizes: %d %d %d\n",
      mpcs[0].data_size, mpcs[1].data_size, mpcs[2].data_size);
  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mpcs), mpcs, &mp) == AC_STATUS_OK);
  error |= AC_TEST(mp != AC_NULL);
  AcMemPool_debug_print("multiple_mem_pool_perf: pool after creation, should have 3 fifo's:", mp);
  ac_debug_printf("\n");

  // Test getting an AcMem
  ac_debug_printf("multiple_mem_pool_perf: first get ac_mem, expecting != AC_NULL\n");
  ac_mem[0] = AcMemPool_get_ac_mem(mp, mpcs[2].data_size);
  error |= AC_TEST(ac_mem[0] != AC_NULL);
  error |= AC_TEST(ac_mem[0]->hdr.user_size == mpcs[2].data_size);
  error |= AC_TEST(ac_mem[0]->data[0] == 0);
  error |= AC_TEST(ac_mem[0]->data[1] == 0);
  error |= AC_TEST(ac_mem[0]->data[2] == 0);
  AcMemPool_debug_print("multiple_mem_pool_perf: pool after first get ac_mem, fifo 2 should now have 2 entries:", mp);
  AcMemPool_ret_ac_mem(ac_mem[0]);
  AcMemPool_debug_print("multiple_mem_pool_perf: pool after returning ac_mem, fifo 2 should now have 3 entries:", mp);
  ac_debug_printf("\n");

  // Test getting all of the memory asking for 1 byte
  ac_debug_printf("multiple_mem_pool_perf: 1 all of the memory, expecting != AC_NULL\n");
  ac_assert(AC_ARRAY_COUNT(mpcs) == 3);
  ac_assert(AC_ARRAY_COUNT(mem) == (mpcs[0].count + mpcs[1].count + mpcs[2].count));
  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(mem); i++) {
    mem[i] = AcMemPool_get_mem(mp, 1);
    error |= AC_TEST(mem[i] != AC_NULL);
    mem[i][0] = i;
  }
  // perf initialized
  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(mem); i++) {
    error |= AC_TEST(mem[i][0] == i);
  }
  AcMemPool_debug_print("multiple_mem_pool_perf: 1 pool after getting all mem, pool is empty:", mp);
  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(mem); i++) {
    AcMemPool_ret_mem(mem[i]);
  }
  AcMemPool_debug_print("multiple_mem_pool_perf: 1 pool after returning all mem, poll is full:", mp);
  ac_debug_printf("\n");

  // Test getting all of the memory a second time
  ac_debug_printf("multiple_mem_pool_perf: 2 all of the memory, expecting != AC_NULL ****\n");
  ac_u32 idx = 0;
  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(mpcs); i++) {
    for (ac_u32 m = 0; m < mpcs[i].count; m++) {
      mem[idx] = AcMemPool_get_mem(mp, 1);
      error |= AC_TEST(mem[idx] != AC_NULL);
      mem[idx][0] = idx;
      idx += 1;
    }
  }
  // perf initialized
  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(mem); i++) {
    error |= AC_TEST(mem[i][0] == i);
  }
  AcMemPool_debug_print("multiple_mem_pool_perf: 2 pool after getting all mem, pool is empty:", mp);
  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(mem); i++) {
    AcMemPool_ret_mem(mem[i]);
  }
  AcMemPool_debug_print("multiple_mem_pool_perf: 2 pool after returning all mem, poll is full:", mp);
  ac_debug_printf("\n");

  ac_debug_printf("multiple_mem_pool_perf:-\n");
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

  error |= simple_mem_pool_perf();
  error |= multiple_mem_pool_perf();
  error |= perf_mem_pool_multiple_threads(1);
  error |= perf_mem_pool_multiple_threads(2);
  error |= perf_mem_pool_multiple_threads(5);
  error |= perf_mem_pool_multiple_threads(10);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
