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
#include <ac_mem_pool/tests/incs/test.h>

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_memset.h>
#include <ac_mpsc_fifo_dbg.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

extern AcMpscFifo* AcMemPool_get_fifo(AcMemPool* pool);

ac_bool simple_mem_pool_test(ac_u32 data_size) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("simple_mem_pool_test:+\n");

  AcMemPool* mp;
  AcMem* mem;
  //AcMem* mem2;
  AC_UNUSED(mem);
  AcMemCountSize mcs[3] = {
    { .count = 1, .data_size = 1 },
    { .count = 2, .data_size = 2 },
    { .count = 3, .data_size = 3 },
  };

  // Testing creating an empty pool returns BAD_PARAM and mp is AC_NULL
  ac_debug_printf("simple_mem_pool_test: create a pool with NO AcMem's\n");
  error |= AC_TEST(AcMemPool_alloc(0, mcs, &mp) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mp == AC_NULL);

  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mcs), AC_NULL, &mp) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mp == AC_NULL);

  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mcs), mcs, AC_NULL) == AC_STATUS_BAD_PARAM);

#if 0
  // Test requesting a message from a AC_NULL pool returns AC_NULL
  ac_debug_printf("simple_mem_pool_test: test an empty pool that AcMem_get returns AC_NULL\n");
  error |= AC_TEST(AcMemPool_get_ac_mem(mp, 1, &mem));
  error |= AC_TEST(mem == AC_NULL);

  // Test returning a AC_NULL mem to a AC_NULL pool doesn't blow up
  ac_debug_printf("simple_mem_pool_test: returning an AC_NULL message doesn't blow up\n");
  AcMem_ret(mem);

  // Testing creating a pool with one mem
  ac_debug_printf("simple_mem_pool_test: create a pool with 1 mem and data_size=%d\n",
      data_size);
  mp = AcMemPool_create_extra(1, data_size);
  error |= AC_TEST(mp != AC_NULL);

#ifndef NDEBUG
  ac_debug_printf("simple_mem_pool_test: pool after creation:\n");
  AcMpscFifo_print(AcMemPool_get_fifo(mp));
#endif

  // Test requesting the message
  ac_debug_printf("simple_mem_pool_test: get mem expecting != AC_NULL\n");
  mem = AcMem_get(mp);
  error |= AC_TEST(mem != AC_NULL);

#ifndef NDEBUG
  ac_debug_printf("simple_mem_pool_test: pool after first get:\n");
  AcMpscFifo_print(AcMemPool_get_fifo(mp));
#endif

  ac_memset(mem->data, 0xa5, data_size);
  for (ac_u32 i = 0; i < data_size; i++) {
    error |= AC_TEST(mem->data[i] == 0xa5);
  }

#ifndef NDEBUG
  ac_debug_printf("simple_mem_pool_test: pool after setting extra data:\n");
  AcMpscFifo_print(AcMemPool_get_fifo(mp));
#endif

  // Test requesting the message which should be empty
  ac_debug_printf("simple_mem_pool_test: get mem expecting AC_NULL\n");
  mem2 = AcMem_get(mp);
  error |= AC_TEST(mem2 == AC_NULL);

  ac_debug_printf("simple_mem_pool_test: return mem=%p\n", mem);
  // Test returning the message we successfully got
  AcMem_ret(mem);

  // Test we can get a message back after its returned
  ac_debug_printf("simple_mem_pool_test: test we can get a mem after returning\n");
  mem = AcMem_get(mp);
  error |= AC_TEST(mem != AC_NULL);

  // Return mem
  AcMem_ret(mem);
#endif

  ac_debug_printf("simple_mem_pool_test:-\n");
  return error;
}

/**
 * main
 */
int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  AcReceptor_init(256);
  AcTime_init();

  ac_debug_printf("sizeof(AcMem)=%d\n", sizeof(AcMem));

  error |= simple_mem_pool_test(0);
  //error |= simple_mem_pool_test(1);
  //error |= simple_mem_pool_test(127);
  //error |= test_mem_pool_multiple_threads(1, 1, 0);
  //error |= test_mem_pool_multiple_threads(1, 10, 1);
  //error |= test_mem_pool_multiple_threads(10, 1, 48);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
