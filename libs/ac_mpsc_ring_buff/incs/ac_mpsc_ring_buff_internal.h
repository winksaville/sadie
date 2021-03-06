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

#ifndef SADIE_LIBS_AC_MPSC_RING_BUFF_INCS_AC_MPSC_RING_BUFF_INTERNAL_H
#define SADIE_LIBS_AC_MPSC_RING_BUFF_INCS_AC_MPSC_RING_BUFF_INTERNAL_H

#include <ac_attributes.h>
#include <ac_cache_line.h>
#include <ac_inttypes.h>
#include <ac_putchar.h>

typedef struct RingBuffCell {
  void* mem;
  AcU32 seq;
} RingBuffCell;


typedef struct AcMpscRingBuff {
  AcU32 add_idx AC_ATTR_ALIGNED(64);
  AcU32 rmv_idx AC_ATTR_ALIGNED(64);
  AcU32 size;
  AcU32 mask;
  RingBuffCell* ring_buffer;

  // Used for debugging and not always valid
  _Atomic(AcU32) count;
  _Atomic(AcU64) processed;
} AcMpscRingBuff;

#endif
