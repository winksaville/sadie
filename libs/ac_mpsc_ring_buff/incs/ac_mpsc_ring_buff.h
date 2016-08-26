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
 * The AcMpscRingBuff is a wait free/thread safe multi-producer
 * single consumer ring buffer. This algorithm is based on Dimitry
 * Vyukov's MPMC bounded queue here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
 */

#ifndef SADIE_LIBS_AC_MPSC_RING_BUFF_INCS_AC_MPSC_RING_BUFF_H
#define SADIE_LIBS_AC_MPSC_RING_BUFF_INCS_AC_MPSC_RING_BUFF_H

#include <ac_mpsc_ring_buff_internal.h>
#include <ac_cache_line.h>
#include <ac_inttypes.h>
#include <ac_mem.h>
#include <ac_putchar.h>

typedef struct AcMpscRingBuff AcMpscRingBuff;


/**
 * Add mem to the ring buffer. This maybe used by multiple
 * threads and never blocks.
 *
 * @params rb is an iniitalized AcMpscRingBuff
 * @params mem is pointing to some arbitrary memory
 *
 * @return AC_TRUE if added AC_FALSE of full
 */
AcBool AcMpscRingBuff_add_mem(AcMpscRingBuff* rb, void* mem);

/**
 * Remove a memory from the ring buffer. This maybe used only by
 * a single thread and returns AC_NULL if the ring buffer is empty.
 *
 * @params rb is an iniitalized AcMpscRingBuff
 *
 * @return The next item or AC_NULL if empty
 */
void* AcMpscRingBuff_rmv_mem(AcMpscRingBuff* rb);

/**
 * Deinitialize the AcMpscRingBuff. Assumes the ring buffer is EMPTY.
 *
 * @params rb is an uniniitalized with AcMpscRingBuff_init_ac_mem_stub
 * @returns number of items added/removed from ring buffer
 */
ac_u64 AcMpscRingBuff_deinit(AcMpscRingBuff* rb);

/**
 * Initialize an AcMpscRingBuff able to manage count items
 *
 * @params rb is an uniniitalized AcMpscRingBuff
 * @params size is the number mem's to allow in the ring buffer
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscRingBuff_init(AcMpscRingBuff* rb, ac_u32 count);

/**
 * Early initialize module
 */
void AcMpscRingBuff_early_init(void);

#endif
