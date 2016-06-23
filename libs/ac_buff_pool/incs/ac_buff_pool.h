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
 * An AcBuffPool manages a set of preallocated buffers created
 * when AcBuffPool_create is called. The AcBuffPool_get_mem may only
 * be invoked by the owner, but any entity may invoke AcBuffPoll_ret_mem.
 */

#ifndef SADIE_LIBS_AC_BUFF_POOL_INCS_AC_BUFF_POOL_H
#define SADIE_LIBS_AC_BUFF_POOL_INCS_AC_BUFF_POOL_H

#include <ac_mpsc_fifo.h>

#include <ac_inttypes.h>
#include <ac_status.h>

typedef struct AcBuffPool AcBuffPool;

typedef struct AcBuffCountSize {
  ac_u32 count;         // Number of mem buffers to create of size
  ac_u32 size;          // Size of each mem buffer in bytes
} AcBuffCountSize;

/**
 * Get an AcBuff with the specified size
 *
 * @param pool is a valid AcBuffPool
 *
 * @return 0 (AC_STATUS_OK) on success
 *         AC_STATUS_OUT_OF_MEMORY if pool is completely empty
 *         AC_sTATUS_NOT_AVAILABLE if pool has nothing that large
 */
AcStatus AcBuffPool_get_ac_buff(AcBuffPool* pool, ac_u32 size, AcBuff** pAcBuf);

/**
 * Get a buffer with the specified size
 *
 * @param pool is a valid AcBuffPool
 *
 * @return a buffer or null if none available
 */
AcStatus AcBuffPool_get_buff(AcBuffPool* pool, ac_u32 size, void** pBuff);

/**
 * Return an AcBuff to its pool
 *
 * @param buff is an AcBuff returned by AcBuffPool_get_ac_buff
 */
void AcBuffPool_ret_ac_buff(AcBuff* buff);

/**
 * Return a buffer to its pool
 *
 * @param buff is a buffer returned by AcBuffPool_get_buff
 */
void AcBuffPool_ret_buff(void* buff);

/**
 * Allocate a memory pool of buffers with the specified size
 *
 * @params count is the size of the bcs array
 *
 * @return 0 (AC_STATUS_OK) if successful
 */
AcStatus AcBuffPool_alloc(ac_u32 count, AcBuffCountSize bcs[], AcBuffPool** pPool);

/**
 * Free the memory pool
 *
 * @params pool is a pool created by AcBuffPool_alloc
 */
void AcBuffPool_free(AcBuffPool* pool);

#endif
