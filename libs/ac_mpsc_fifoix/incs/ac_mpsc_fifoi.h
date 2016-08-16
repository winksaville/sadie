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
 * A MpscFifo is a wait free/thread safe multi-producer
 * single consumer first in first out queue. This algorithm
 * is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 *
 * The fifo has a head and tail, This is an "intrusive" algorithm
 * so each element, an AcMem element must include an next field
 * as the first field.
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H

#include <ac_cache_line.h>
#include <ac_mem.h>
#include <ac_msg.h>
#include <ac_putchar.h>

typedef struct AcMpscFifoI {
  /** head of the list where items are added */
  AcMem* head __attribute(( aligned(AC_MAX_CACHE_LINE_LEN) ));

  /** tail->next is the next AcMem to
    * be removed (tail->next is AC_NULL if empty) */
  AcMem* tail __attribute(( aligned(AC_MAX_CACHE_LINE_LEN) ));

  /** AcMem's owned by this fifo */
  AcMem* mem_array;

  /** Stub added to the fifo when its empty */
  AcMem stub;

  ac_u32 count;
  ac_u64  msgs_processed;
} AcMpscFifoI;


/**
 * Add a AcMem to the fifo. This maybe used by multiple
 * threads and never blocks.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 * @params acmem is an AcMem
 */
void AcMpscFifoI_add_ac_mem(AcMpscFifoI* fifo, AcMem* ac_mem);

/**
 * Add a AcMem to the fifo. This maybe used by multiple
 * threads and never blocks.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 * @params mem is pointing at the AcMem.data array
 */
static inline void AcMpscFifoI_add_mem(AcMpscFifoI* fifo, void* mem) {
  AcMpscFifoI_add_ac_mem(fifo, (AcMem*)(mem - sizeof(AcMemHdr)));
}

/**
 * Add a AcMsg to the fifo. This maybe used by multiple
 * threads and never blocks.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 * @params msg is pointing at the AcMem.data array
 */
static inline void AcMpscFifoI_add_msg(AcMpscFifoI* fifo, AcMsg* msg) {
  AcMpscFifoI_add_mem(fifo, msg);
}

/**
 * Remove a AcMem from the FIFO. This maybe used only by
 * a single thread and returns AC_NULL if the FIFO is empty.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 *
 * @return AcMem or AC_NULL if empty
 */
AcMem* AcMpscFifoI_rmv_ac_mem(AcMpscFifoI* fifo);

/**
 * Remove a raw AcMem from the FIFO. A raw memer contents
 * are NOT preserved and the caller will be initializing the
 * contents after removal. This is useful in managing a pool
 * of empty memers.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 *
 * @return AcMem or AC_NULL if empty
 */
AcMem* AcMpscFifoI_rmv_ac_mem_raw(AcMpscFifoI* fifo);

/**
 * Remove a memory block from the FIFO. This maybe used only by
 * a single thread and returns AC_NULL if the FIFO is empty.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 *
 * @return AcMem or AC_NULL if empty
 */
static inline void* AcMpscFifoI_rmv_mem(AcMpscFifoI* fifo) {
  AcMem* ac_mem = AcMpscFifoI_rmv_ac_mem(fifo);
  if (ac_mem != AC_NULL) {
    return  &ac_mem->data[0];
  } else {
    return AC_NULL;
  }
}

/**
 * Remove an AcMsg from the FIFO. This maybe used only by
 * a single thread and returns AC_NULL if the FIFO is empty.
 *
 * @params fifo is an iniitalized AcMpscFifoI
 *
 * @return AcMem or AC_NULL if empty
 */
static inline AcMsg* AcMpscFifoI_rmv_msg(AcMpscFifoI* fifo) {
  return (AcMsg*)AcMpscFifoI_rmv_mem(fifo);
}


/**
 * Deinitialize the AcMpscFifoI. Assumes the fifo is EMPTY.
 *
 * @params fifo is an iniitalized AcMpscFifoI_init or AcMpscFifoI_init_and_alloc
 */
void AcMpscFifoI_deinit(AcMpscFifoI* fifo);

/**
 * Deinitialize the AcMpscFifoI, assumes the fifo is FULL.
 *
 * @params fifo is an iniitalized AcMpscFifoI_init_and_alloc
 */
void AcMpscFifoI_deinit_full(AcMpscFifoI* fifo);

/**
 * Initialize an AcMpscFifoI allocating AcMem's that
 * will added and owned by the fifo
 *
 * @params fifo is an uniniitalized AcMpscFifoI
 * @params count is the number of AcMem's to allocate and add
 *         (There will always be a stub allocated)
 * @params data_size is the size of Acmem.data array for each AcMem
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifoI_init_and_alloc(AcMpscFifoI* fifo, ac_u32 count,
    ac_u32 data_size);

/**
 * Initialize an AcMpscFifoI with a stub allocated with AcMemPool_get_ac_mem
 *
 * @params fifo is an uniniitalized AcMpscFifoI
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifoI_init(AcMpscFifoI* fifo);

/**
 * Early initialize module
 */
void AcMpscFifoI_early_init(void);

#endif
