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

#include <ac_mpsc_ring_buff.h>
#include <ac_mpsc_ring_buff_dbg.h>
#include <ac_mpsc_ring_buff_internal.h>

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
 * @see ac_mpsc_ring_buff.h
 */
AcBool AcMpscRingBuff_add_mem(AcMpscRingBuff* rb, void* mem) {
  ac_debug_printf("AcMpscRingBuff_add_mem:+rb=%p mem=%p\n", rb, mem);

  if (mem != AC_NULL) {
    RingBuffCell* cell;
    AcU32 pos = rb->add_idx;

    while (AC_TRUE) {
      cell = &rb->ring_buffer[pos & rb->mask];
      AcU32 seq = __atomic_load_n(&cell->seq, __ATOMIC_ACQUIRE);
      ac_s32 dif = seq - pos;

      if (dif == 0) {
        if (__atomic_compare_exchange_n((AcU32*)&rb->add_idx, &pos, pos + 1,
              AC_TRUE, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
          break;
        }
      } else if (dif < 0) {
        ac_debug_printf("AcMpscRingBuff_add_mem:-rb=%p mem=%p FULL\n", rb, mem);
        return AC_FALSE;
      } else {
        pos = rb->add_idx;
      }
    }

    cell->mem = mem;
    __atomic_store_n(&cell->seq, pos + 1, __ATOMIC_RELEASE);
#ifndef NDEBUG
    rb->count += 1;
#endif
  }

  ac_debug_printf("AcMpscRingBuff_add_mem:-rb=%p mem=%p\n", rb, mem);
  return AC_TRUE;
}

/**
 * @see ac_mpsc_ring_buff.h
 */
void* AcMpscRingBuff_rmv_mem(AcMpscRingBuff* rb) {
  ac_debug_printf("AcMpscRingBuff_rmv_mem:+rb=%p\n", rb);
  void* mem;
  RingBuffCell* cell;
  AcU32 pos = rb->rmv_idx;

  cell = &rb->ring_buffer[pos & rb->mask];
  AcU32 seq = __atomic_load_n(&cell->seq, __ATOMIC_ACQUIRE);
  ac_s32 dif = seq - (pos + 1);

  if (dif < 0) {
    ac_debug_printf("AcMpscRingBuff_rmv_mem:-rb=%p mem=AC_NULL EMPTY\n", rb);
    return AC_NULL;
  }
  
  mem = cell->mem;
  __atomic_store_n(&cell->seq, pos + rb->size, __ATOMIC_RELEASE);

  rb->rmv_idx += 1;
#ifndef NDEBUG
  rb->processed += 1;
  rb->count -= 1;
#endif

  ac_debug_printf("AcMpscRingBuff_rmv_mem:-rb=%p mem=%p\n", rb, mem);
  return mem;
}

/**
 * @see ac_mpsc_ring_buff.h
 */
AcU64 AcMpscRingBuff_deinit(AcMpscRingBuff* rb) {
  ac_debug_printf("AcMpscRingBuff_deinit:+rb=%p\n", rb);

  AcU64 processed = rb->processed;
  ac_free(rb->ring_buffer);
  rb->ring_buffer = AC_NULL;
  rb->add_idx = 0;
  rb->rmv_idx = 0;
  rb->size = 0;
  rb->mask = 0;
  rb->count = 0;
  rb->processed = 0;

  ac_debug_printf("AcMpscRingBuff_deinit:-rb=%p processed=%lu\n", rb, processed);
 return processed;
}

/**
 * @see ac_mpsc_ring_buff.h
 */
AcStatus AcMpscRingBuff_init(AcMpscRingBuff* rb, AcU32 size) {
  AcStatus status = AC_STATUS_OK;

  ac_debug_printf("AcMpscRingBuff_init:+rb=%p size=%d\n", rb, size);

  if (rb == AC_NULL) {
    ac_debug_printf("AcMpscRingBuff_init:-rb=%p size=%d rb is AC_NULL return BAD_PARAM\n",
        rb, size);
    return AC_STATUS_BAD_PARAM;
  }

  rb->add_idx = 0;
  rb->rmv_idx = 0;
  AcU32 one_bits = 0;
  AcU32 test = size;
  for (AcU32 i = 0; i < (sizeof(size) * 8); i++) {
    one_bits += test & 1;
    test >>= 1;
  }
  if (one_bits != 1) {
    ac_debug_printf("AcMpscRingBuff_init:-rb=%p size=%d not a power of 2 return BAD_PARAM\n",
        rb, size);
    return AC_STATUS_BAD_PARAM;
  }

  rb->size = size;
  rb->mask = size - 1;
  rb->processed = 0;
  rb->count = 0;
  rb->ring_buffer = ac_malloc(size * sizeof(*rb->ring_buffer));
  for (AcU32 i = 0; i < rb->size; i++) {
    rb->ring_buffer[i].seq = i;
    rb->ring_buffer[i].mem = AC_NULL;
  }
  if (rb->ring_buffer == AC_NULL) {
    ac_debug_printf("initMpscRingBuff:-pRb=%p size=%d could not allocate ring_buffer return OUT_OF_MEMORY\n",
        rb, size);
    return AC_STATUS_OUT_OF_MEMORY;
  }

  ac_debug_printf("AcMpscRingBuff_init:-rb=%p size=%d status=%d\n", rb, size, status);
  return status;
}


/**
 * Early initialize module (deprecated????)
 */
__attribute__((constructor))
void AcMpscRingBuff_early_init(void) {
}
