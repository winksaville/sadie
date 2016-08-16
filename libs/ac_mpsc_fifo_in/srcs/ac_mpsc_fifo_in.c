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

#ifdef NDEBUG
#define COUNT
#endif

#include <ac_mpsc_fifo_in.h>
#include <ac_mpsc_fifo_in_dbg.h>
#include <ac_mpsc_fifo_in_internal.h>

#include <ac_assert.h>
#include <ac_mem_dbg.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_memcpy.h>
#include <ac_debug_printf.h>
#include <ac_thread.h>
#include <ac_tsc.h>

#define SEQ_CST 0 // 1 for memory order to be __ATOMIC_SEQ_CST for all atomics

/**
 * @see ac_mpsc_fifo_in.h
 */
void AcMpscFifoIn_add_ac_mem(AcMpscFifoIn* fifo, AcMem* mem) {
  ac_debug_printf("AcMpscFifoIn_add_ac_mem:+fifo=%p mem=%p\n", fifo, mem);

  mem->hdr.next = AC_NULL;
  AcMem* prev = __atomic_exchange_n(&fifo->head, mem, __ATOMIC_ACQ_REL);
  // rmv will stall spinning if preempted at this critical spot
  __atomic_store_n(&prev->hdr.next, mem, __ATOMIC_RELEASE);

#ifndef COUNT
  fifo->processed += 1;
  fifo->count += 1;
#endif

  ac_debug_printf("AcMpscFifoIn_add_ac_mem:-fifo=%p mem=%p\n", fifo, mem);
}

/**
 * @see ac_mpsc_fifo_in.h
 */
AcMem* AcMpscFifoIn_rmv_ac_mem(AcMpscFifoIn* fifo) {
  ac_debug_printf("AcMpscFifoIn_rmv_ac_mem:+fifo=%p\n", fifo);

  // Not using __ATOMIC types and streamlined with no comments or debug
  AcMem* tail = fifo->tail;
  AcMem* next = __atomic_load_n(&tail->hdr.next, __ATOMIC_ACQUIRE);
  if (tail == &fifo->stub) {
    if (next == AC_NULL) {
      return AC_NULL;
    }
    fifo->tail = next;
    tail = next;
    next = __atomic_load_n(&tail->hdr.next, __ATOMIC_ACQUIRE);
  }

  if (next == AC_NULL) {
    AcMem* head = __atomic_load_n(&fifo->head, __ATOMIC_ACQUIRE);
    if (tail == head) {
      AcMpscFifoIn_add_ac_mem(fifo, &fifo->stub);
    }

    next = __atomic_load_n(&tail->hdr.next, __ATOMIC_ACQUIRE);
    while (next == AC_NULL) {
      ac_thread_yield();
      next = __atomic_load_n(&tail->hdr.next, __ATOMIC_ACQUIRE);
    }
  }

#ifndef COUNT
  fifo->processed += 1;
  fifo->count -= 1;
#endif

  fifo->tail = next;

  ac_debug_printf("AcMpscFifoIn_rmv_ac_mem:-fifo=%p mem=%p\n", fifo, tail);
  return tail;
}

/**
 * @see ac_mpsc_ring_buff.h
 */
ac_u64 AcMpscFifoIn_deinit(AcMpscFifoIn* fifo) {
  ac_debug_printf("AcMpscFifoIn_deinit:+fifo=%p\n", fifo);

  ac_u64 processed = fifo->processed;
  fifo->head = AC_NULL;
  fifo->tail = AC_NULL;
  fifo->count = 0;
  fifo->processed = 0;

  ac_debug_printf("AcMpscFifoIn_deinit:-fifo=%p processed=%lu\n", fifo, processed);
 return processed;
}

/**
 * @see ac_mpsc_ring_buff.h
 */
AcStatus AcMpscFifoIn_init(AcMpscFifoIn* fifo) {
  AcStatus status = AC_STATUS_OK;

  ac_debug_printf("AcMpscFifoIn_init:+fifo=%p\n", fifo);

  fifo->head = &fifo->stub;
  fifo->tail = &fifo->stub;
  fifo->count = 0;
  fifo->processed = 0;
  fifo->stub.hdr.next = AC_NULL;
  fifo->stub.hdr.pool_fifo = AC_NULL;
  fifo->stub.hdr.data_size = 0;
  fifo->stub.hdr.user_size = 0;

  ac_debug_printf("AcMpscFifoIn_init:-fifo=%p status=%d\n", fifo, status);
  return status;
}


/**
 * Early initialize module (deprecated????)
 */
__attribute__((constructor))
void AcMpscFifoIn_early_init(void) {
}
