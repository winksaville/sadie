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
 * The AcMpscFifoIn is a wait free/thread safe multi-producer
 * single consumer ring buffer. This algorithm is based on Dimitry
 * Vyukov's MPMC bounded queue here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
 */

#ifndef SADIE_LIBS_AC_MPSC_FIFO_IN_INCS_AC_MPSC_FIFO_IN_H
#define SADIE_LIBS_AC_MPSC_FIFO_IN_INCS_AC_MPSC_FIFO_IN_H

#include <ac_cache_line.h>
#include <ac_inttypes.h>
#include <ac_mem.h>
#include <ac_putchar.h>

typedef struct AcMpscFifoIn AcMpscFifoIn;


/**
 * Add mem to the ring buffer. This maybe used by multiple
 * threads and never blocks.
 *
 * @params fifo is an iniitalized AcMpscFifoIn
 * @params mem is pointing to some arbitrary memory
 */
void AcMpscFifoIn_add_ac_mem(AcMpscFifoIn* fifo, AcMem* mem);

/**
 * Remove a memory from the ring buffer. This maybe used only by
 * a single thread and returns AC_NULL if the ring buffer is empty.
 *
 * @params fifo is an iniitalized AcMpscFifoIn
 *
 * @return The next item or AC_NULL if empty
 */
AcMem* AcMpscFifoIn_rmv_ac_mem(AcMpscFifoIn* fifo);

/**
 * Deinitialize the AcMpscFifoIn. Assumes the fifo is EMPTY.
 *
 * @params fifo is an uniniitalized with AcMpscFifoIn_init_ac_mem_stub
 *
 * @returns number of items added/removed from the fifo
 */
ac_u64 AcMpscFifoIn_deinit(AcMpscFifoIn* fifo);

/**
 * Initialize an AcMpscFifoIn able to manage count items
 *
 * @params fifo is an uniniitalized AcMpscFifoIn
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifoIn_init(AcMpscFifoIn* fifo);

/**
 * Early initialize module
 */
void AcMpscFifoIn_early_init(void);

#endif
