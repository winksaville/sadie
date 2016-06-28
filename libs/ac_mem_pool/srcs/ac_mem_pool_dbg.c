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

/**
 * Debug code for mpscpool
 */

#define NDEBUG

#include <ac_mem_pool.h>
#include <ac_mem_pool_dbg.h>
#include <ac_mem_pool_internal.h>

#include <ac_mpsc_fifo_dbg.h>
#include <ac_printf.h>

/**
 * @see AcMemPool_dbg.h
 */
void AcMemFifo_print(const char* leader, AcMemFifo* mem_fifo) {
  if (mem_fifo != AC_NULL) {
    if (leader == AC_NULL) {
      ac_printf("mem_fifo: %p ", mem_fifo);
    } else {
      ac_printf("%s %p ", leader, mem_fifo);
    }
    ac_printf("count=%d data_size=%d mem_array=%p\n",
        mem_fifo->mcs.count, mem_fifo->mcs.data_size, mem_fifo->mem_array);
    AcMpscFifo_print(AC_NULL, &mem_fifo->fifo);
  }
}

/**
 * @see AcMemPool_dbg.h
 */
void AcMemPool_print(const char* leader, AcMemPool* pool) {
  if (pool != AC_NULL) {
    if (leader == AC_NULL) {
      ac_printf("pool=%p\n", pool);
    } else {
      ac_printf("%s %p\n", leader, pool);
    }
    ac_printf("count=%d\n", pool->count);
    for (ac_u32 i = 0; i < pool->count; i++) {
      AcMemFifo_print(AC_NULL, &pool->mem_fifo_array[0]);
    }
  }
}
