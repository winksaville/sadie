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
 * An AcMemPool manages a set of preallocated AcMem's created
 * with AcMemPool_alloc. The AcMemPool_get_mem may only be invoked by
 * the owner, but any entity may invoke AcMemPoll_ret_mem.
 */

#ifndef SADIE_LIBS_AC_MEM_POOL_INCS_AC_MEM_POOL_H
#define SADIE_LIBS_AC_MEM_POOL_INCS_AC_MEM_POOL_H

#include <ac_mem.h>
#include <ac_mpsc_fifo.h>

#include <ac_inttypes.h>
#include <ac_status.h>

typedef struct AcMemPool AcMemPool;

typedef struct AcMemPoolCountSize {
  ac_u32 count;         // Number of mem AcMem's to create
  ac_u32 data_size;     // Size of each of the AcMem.data[]
} AcMemPoolCountSize;

/**
 * Get an AcMem with the specified sized AcMem.data
 *
 * @param pool is a valid AcMemPool
 * @param size is length of the AcMem.data field
 *
 * @return AcMem* or AC_NULL if an error.
 */
AcMem* AcMemPool_get_ac_mem(AcMemPool* pool, ac_u32 size);

/**
 * Return an AcMem to its pool
 *
 * @param mem is an AcMem returned by AcMemPool_get_ac_mem
 */
void AcMemPool_ret_ac_mem(AcMem* mem);

/**
 * Get memory with the specified size
 *
 * @param pool is a valid AcMemPool
 * @param size is length of the memory to retrive
 *
 * @return Pointer to memory that is size in length
 */
void* AcMemPool_get_mem(AcMemPool* pool, ac_u32 size);

/**
 * Ret the memory to its pool.
 *
 * @param mem is a pointer to memory previously returned by AcMemPool_get_mem
 */
void AcMemPool_ret_mem(void* mem);

/**
 * Allocate a memory pool of various sized AcMem's. Each AcMemPoolCountSize
 * element determines the number of AcMem's to create and the size of
 * each of there AcMem.data arrays.
 *
 * @params count is the size of the mcs array
 * @params mpcs is an array of AcMemPoolCountSize's defining the of each AcMem
 *         and there count.
 * @params ptr_pool is an out parameter pointing to the pool new created pool
 *
 * @return 0 (AC_STATUS_OK) if successful
 */
AcStatus AcMemPool_alloc(ac_u32 count, AcMemPoolCountSize mpcs[],
    AcMemPool** ptr_pool);

/**
 * Free the memory pool and all of its associated AcMem objects
 *
 * @params pool is a pool created by AcMemPool_alloc
 */
void AcMemPool_free(AcMemPool* pool);

#endif
