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

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_mem.h>
#include <ac_memmgr.h>
#include <ac_mpsc_fifo.h>
#include <ac_status.h>

/**
 * Get an AcMem with the specified sized AcMem.data
 *
 * @param pool is a valid AcMemPool
 *
 * @return 0 (AC_STATUS_OK) on success *ptr_AcMem is AcMem* or AC_NULL
 *         AC_STATUS_BAD_PARAM if pool is AC_NULL
 *         AC_STATUS_NOT_AVAILABLE if pool has nothing that large
 */
AcStatus AcMemPool_get_ac_mem(AcMemPool* pool, ac_u32 size, AcMem** ptr_AcMem) {
  AcStatus status;
  AcMem* mem;

  ac_debug_printf("AcMemPool_get:+pool=%p\n", pool);

  if ((pool == AC_NULL) || (size == 0) || (ptr_AcMem == AC_NULL)) {
    ac_debug_printf("AcMemPool_get: err bad param "
        "pool=%p == AC_NULL or size=%d == 0 or ptr_AcMem=%p AC_NULL\n",
        pool, size, ptr_AcMem);
    status = AC_STATUS_BAD_PARAM;
    mem = AC_NULL;
    goto done;
  }

  for (ac_u32 i = 0; i < pool->count; i++) {
    AcMemFifo* mem_fifo = &pool->mem_fifo_array[i];

    // Find first fit
    if (mem_fifo->mpcs.data_size >= size) {
      mem = AcMpscFifo_rmv_ac_mem_raw(&mem_fifo->fifo);
      if (mem != AC_NULL) {
        // Got one
        mem->hdr.user_size = size;
        status = AC_STATUS_OK;
        goto done;
      }
    }
  }

  // We didn't find an AcMem
  status = AC_STATUS_NOT_AVAILABLE;
  mem = AC_NULL;

done:
  ac_debug_printf("AcMemPool_get:-pool=%p mem=%p status=%d\n", pool, mem, status);
  if (ptr_AcMem != AC_NULL) {
    *ptr_AcMem = mem;
  }
  return status;
}


/**
 * Return an AcMem to its pool
 *
 * @param mem is an AcMem returned by AcMemPool_get_ac_mem
 */
void AcMemPool_ret_ac_mem(AcMem* mem) {
  ac_debug_printf("AcMemPool_ret_ac_mem:+mem=%p\n", mem);

  if (mem != AC_NULL) {
    mem->hdr.user_size = 0;
    AcMem_ret(mem);
  }

  ac_debug_printf("AcMemPool_ret_ac_mem:-mem=%p\n", mem);
}

/**
 * Get memory with the specified size
 *
 * @param pool is a valid AcMemPool
 *
 * @return 0 (AC_STATUS_OK) on success *ptr_mem is &AcMem.data[0] or AC_NULL
 *         AC_STATUS_BAD_PARAM if pool is AC_NULL
 *         AC_STATUS_NOT_AVAILABLE if pool has nothing that large
 */
AcStatus AcMemPool_get_mem(AcMemPool* pool, ac_u32 size, void** ptr_mem) {
  AcStatus status;
  void* data;
  ac_debug_printf("AcMemPool_get_mem:+pool=%p\n", pool);

  if ((pool == AC_NULL) || (ptr_mem == AC_NULL)) {
    ac_debug_printf("AcMemPool_get_mem: err AC_NULL pool=%p or ptr_mem=%p\n", pool, ptr_mem);
    status = AC_STATUS_BAD_PARAM;
    data = AC_NULL;
    goto done;
  }

  AcMem* mem;
  status = AcMemPool_get_ac_mem(pool, size, &mem);
  if (status == AC_STATUS_OK) {
    data = mem->data;
  } else {
    data = AC_NULL;
  }

done:
  ac_debug_printf("AcMemPool_get_mem:-pool=%p status=%d mem=%p\n", pool, status, data);
  if (ptr_mem != AC_NULL) {
    *ptr_mem = data;
  }
  return status;
}

/**
 * Ret the memory to its pool.
 *
 * @param mem is a pointer to memory previously returned by AcMemPool_get
 */
void AcMemPool_ret_mem(void* mem) {
  ac_debug_printf("AcMemPool_ret:+mem=%p\n", mem);

  if (mem != AC_NULL) {
    AcMemPool_ret_ac_mem((AcMem*)(mem - sizeof(AcMem)));
  }

  ac_debug_printf("AcMemPool_ret:-mem=%p\n", mem);
}


/**
 * Allocate a memory pool of various sized AcMem's. Each AcMemCountSize
 * element determines the number of AcMem's to create and the size of
 * each of there AcMem.data arrays.
 *
 * @params count is the size of the mpcs array
 * @params mpcs is an array of AcMemCountSize's defining the of each AcMem
 *         and there count.
 * @params ptr_pool is an out parameter pointing to the pool new created pool
 *
 * @return 0 (AC_STATUS_OK) if successful
 */
AcStatus AcMemPool_alloc(ac_u32 count, AcMemPoolCountSize mpcs[],
    AcMemPool** ptr_pool) {
  AcStatus status;
  AcMemPool* pool;
  
  ac_debug_printf("AcMemPool_alloc:+count=%d mpcs=%p\n", count, mpcs);

  if ((count == 0) || (mpcs == AC_NULL) || (ptr_pool == AC_NULL)) {
    ac_debug_printf("AcMemPool_alloc: err bad param"
        " count=%d is 0 or mpcs=%p = AC_NULL or ptr_pool=%p == AC_NULL\n",
        count, mpcs, ptr_pool);
    pool = AC_NULL;
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  // Allocate the AcMemPool filled with 0
  pool = (AcMemPool*)ac_calloc(1, sizeof(AcMemPool) + (count * sizeof(AcMemFifo)));
  if (pool == AC_NULL) {
    ac_debug_printf("AcMemPool_alloc: err count is 0 return pool=AC_NULL\n");
    status = AC_STATUS_OUT_OF_MEMORY;
    goto done;
  }
  pool->count = count;


  // Initialize the fifo's allocating them to this fifo
  for (ac_u32 i = 0; i < pool->count; i++) {
    AcMemFifo* mem_fifo = &pool->mem_fifo_array[i];

    // Init mem count and size
    mem_fifo->mpcs = mpcs[i];

    // Init the fifo
    status = AcMpscFifo_init_and_alloc(&mem_fifo->fifo, mem_fifo->mpcs.count, mem_fifo->mpcs.data_size);
    if (status != AC_STATUS_OK) {
      ac_debug_printf("AcMemPool_alloc: err init_and_alloc failed status=%d\n", status);
      goto done;
    }
  }

  // Successful
  status = AC_STATUS_OK;

done:
  if (status != AC_STATUS_OK) {
    AcMemPool_free(pool);
  }

  if (ptr_pool != AC_NULL) {
    *ptr_pool = pool;
  }

  AcMemPool_debug_print("AcMemPool_alloc:", pool);

  ac_debug_printf("AcMemPool_alloc:-count=%d mpcs=%p pool=%p status=%d\n", count, mpcs, pool, status);
  return status;
}

/**
 * Free the memory pool and all of its associated AcMem objects
 *
 * @params pool is a pool created by AcMemPool_alloc
 */
void AcMemPool_free(AcMemPool* pool) {
  if (pool != AC_NULL) {
    for (ac_u32 i = 0; i < pool->count; i++) {
      AcMemFifo* mem_fifo = &pool->mem_fifo_array[i];

      if (mem_fifo->mem_array != AC_NULL) {
        // Deinit the fifo, it better be full
        AcMpscFifo_deinit_full(&mem_fifo->fifo);
      }
    }
    ac_free(pool);
  }
}
