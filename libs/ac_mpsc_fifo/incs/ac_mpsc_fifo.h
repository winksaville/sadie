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
 * The fifo has a head and tail, the AcMem elements are added
 * to the head of the and removed from the tail.
 * To allow for a wait free algorithm a there is an extra AcMem,
 * called the stub, so that a single atomic instruction can be
 * used to add and remove an AcMem. The AcMpscFifo_deinit must
 * be called so that the stub maybe returned to is proper pool.
 *
 * A consequence of this algorithm is that when you add an
 * AcMem to the queue a different AcMem is returned when
 * you remove it from the queue. Of course the contents are
 * the same, but the returned pointer will be different.
 *
 * ...
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H

#include <ac_mem.h>
#include <ac_putchar.h>

#define FIFO_USE_REGULAR_TYPES 0
#define FIFO_USE_ATOMIC_TYPES (!FIFO_USE_REGULAR_TYPES)
#define FIFO_ALIGN_TO_64 0

typedef struct AcMpscFifo {
#if FIFO_USE_REGULAR_TYPES & FIFO_ALIGN_TO_64
  AcMem* head __attribute(( aligned(64) )); // head of the list where items are added
  AcMem* tail __attribute(( aligned(64) )); // tail->next is the next AcMem to
                     // be removed (tail->next is AC_NULL if empty)
#elif FIFO_USE_REGULAR_TYPES
  AcMem* head;       // head of the list where items are added
  AcMem* tail;       // tail->next is the next AcMem to
                     // be removed (tail->next is AC_NULL if empty)
#elif FIFO_USE_ATOMIC_TYPES & FIFO_ALIGN_TO_64
  _Atomic(AcMem*) head __attribute(( aligned(64) )); // head of the list where items are added
  _Atomic(AcMem*) tail __attribute(( aligned(64) )); // tail->next is the next AcMem to
                     // be removed (tail->next is AC_NULL if empty)
#elif FIFO_USE_ATOMIC_TYPES
  _Atomic(AcMem*) head;       // head of the list where items are added
  _Atomic(AcMem*) tail;       // tail->next is the next AcMem to
                     // be removed (tail->next is AC_NULL if empty)
#else
  error "BAD ac_mpsc_fifo configuration"
#endif
  AcMem* mem_array;  // AcMem's owned by this fifo, there is
                     // always at least one, the stub.
  ac_u32 count;      // Number of AcMems owned by this fifo
} AcMpscFifo;

#define CRASH() do { (*(volatile ac_u8*)0) = 0; } while(AC_FALSE)

extern volatile char ck_ch;

inline static void CKF(AcMpscFifo* fifo, char ch) {
  if (fifo != AC_NULL) {
    if (fifo->tail != AC_NULL) {
      ac_u32 low24 = (ac_u32)((ac_u64)fifo->tail & 0xffffff);
      if (low24 == 0) {
        ck_ch = ch;
        CRASH();
      } else {
        ac_u32 tailhi = (ac_u32)(((ac_u64)fifo->tail >> 24) & 0xffffffffff);
        ac_u32 headhi = (ac_u32)(((ac_u64)fifo->head >> 24) & 0xffffffffff);
        if (tailhi != headhi) {
          ck_ch = ch;
          CRASH();
        }
      }
    }
  }
}

inline static void CKM(AcMpscFifo* fifo, volatile AcMem* m, char ch) {
  if (fifo != AC_NULL) {
    if (fifo->head != AC_NULL && m != AC_NULL) {
      ac_u32 m_low24 = (ac_u32)((ac_u64)m & 0xffffff);
      if (m_low24 == 0) {
        ck_ch = ch;
        CRASH();
      } else {
        ac_u32 m_hi = (ac_u32)(((ac_u64)m >> 24) & 0xffffffffff);
        ac_u32 headhi = (ac_u32)(((ac_u64)fifo->head >> 24) & 0xffffffffff);
        if (m_hi != headhi) {
        ck_ch = ch;
        CRASH();
        }
      }
    }
  }
}

inline static ac_bool CKM_OK(AcMpscFifo* fifo, volatile AcMem* m, char ch) {
  if (fifo != AC_NULL) {
    if (fifo->head != AC_NULL && m != AC_NULL) {
      ac_u32 m_low24 = (ac_u32)((ac_u64)m & 0xffffff);
      if (m_low24 == 0) {
        return AC_FALSE;
      } else {
        ac_u32 m_hi = (ac_u32)(((ac_u64)m >> 24) & 0xffffffffff);
        ac_u32 headhi = (ac_u32)(((ac_u64)fifo->head >> 24) & 0xffffffffff);
        if (m_hi != headhi) {
        return AC_FALSE;
        }
      }
    }
  }
  return AC_TRUE;
}

/**
 * Add a AcMem to the fifo. This maybe used by multiple
 * threads and never blocks.
 *
 * @params fifo is an iniitalized AcMpscFifo
 */
void AcMpscFifo_add_ac_mem(AcMpscFifo* fifo, AcMem* mem);

/**
 * Remove a AcMem from the FIFO. This maybe used only by
 * a single thread and returns AC_NULL if the FIFO is empty.
 *
 * @params fifo is an iniitalized AcMpscFifo
 *
 * @return AcMem or AC_NULL if empty
 */
AcMem* AcMpscFifo_rmv_ac_mem(AcMpscFifo* fifo);

/**
 * Remove a raw AcMem from the FIFO. A raw memer contents
 * are NOT preserved and the caller will be initializing the
 * contents after removal. This is useful in managing a pool
 * of empty memers.
 *
 * @params fifo is an iniitalized AcMpscFifo
 *
 * @return AcMem or AC_NULL if empty
 */
AcMem* AcMpscFifo_rmv_ac_mem_raw(AcMpscFifo* fifo);

/**
 * Deinitialize the AcMpscFifo and return the stub which
 * needs to be disposed of properly. Assumes the fifo is empty.
 *
 * @params fifo is an iniitalized AcMpscFifo
 */
void AcMpscFifo_deinit(AcMpscFifo* fifo);

/**
 * Deinitialize the AcMpscFifo and return the stub which
 * needs to be disposed of properly. Assumes the fifo is full.
 *
 * @params fifo is an iniitalized AcMpscFifo
 */
void AcMpscFifo_deinit_full(AcMpscFifo* fifo);

/**
 * Initialize an AcMpscFifo allocating AcMem's that
 * will added and owned by the fifo
 *
 * @params fifo is an uniniitalized AcMpscFifo
 * @params count is the number of AcMem's to allocate and add
 *         (There will always be a stub allocated)
 * @params data_size is the size of Acmem.data array for each AcMem
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifo_init_and_alloc(AcMpscFifo* fifo, ac_u32 count,
    ac_u32 data_size);

/**
 * Initialize an AcMpscFifo.
 *
 * @params fifo is an uniniitalized AcMpscFifo
 * @params mem_size number of bytes in AcMem.data[]
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifo_init(AcMpscFifo* fifo, ac_u32 mem_size);

/**
 * Early initialize module
 */
void AcMpscFifo_early_init(void);

#endif
