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
 * Debug code for mpscrb
 */

#define NDEBUG

#include <ac_mpsc_ring_buff.h>
#include <ac_mpsc_ring_buff_dbg.h>
#include <ac_mpsc_ring_buff_internal.h>

#include <ac_printf.h>

/**
 * @see AcMpscRingBuff_dbg.h
 */
void AcMpscRingBuff_print(const char* leader, AcMpscRingBuff* rb) {
  if (rb != AC_NULL) {
    if (leader != AC_NULL) {
      ac_printf("%s\n", leader);
    }

    AcU32 add_idx = rb->add_idx;
    AcU32 rmv_idx = rb->rmv_idx;

    ac_printf("rb=%p add_idx=%d rmv_idx=%d count=%d processed=%lu ",
        rb, add_idx, rmv_idx, rb->count, rb->processed);

    AcU32 pos = rmv_idx;
    RingBuffCell* cell = &rb->ring_buffer[pos & rb->mask];
    AcU32 seq = cell->seq;
    ac_s32 diff = seq - (pos + 1);
    if (diff < 0) {
        ac_printf("empty\n");
    } else {
      ac_printf("\n");
      while (diff >= 0) {
        ac_printf(" cell[%d].seq=%d .mem=%p\n", pos & rb->mask, seq, cell->mem);
        pos += 1;
        cell = &rb->ring_buffer[pos & rb->mask];
        seq = cell->seq;
        diff = seq - (pos + 1);
      }
    }
  } else {
    ac_printf("rb == AC_NULL");
  }
}
