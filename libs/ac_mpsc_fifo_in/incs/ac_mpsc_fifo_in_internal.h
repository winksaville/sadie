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

#ifndef SADIE_LIBS_AC_MPSC_FIFO_IN_INCS_AC_MPSC_FIFO_IN_INTERNAL_H
#define SADIE_LIBS_AC_MPSC_FIFO_IN_INCS_AC_MPSC_FIFO_IN_INTERNAL_H

#include <ac_cache_line.h>
#include <ac_inttypes.h>
#include <ac_mem.h>

typedef struct AcMpscFifoIn {
  AcMem* head __attribute__(( aligned (64) ));
  AcMem* tail __attribute__(( aligned (64) ));
  struct { // anonymous struct because AcAem ends variable size "ac_u8 data[]"
    AcMem  stub;
  };

  // Used for debugging and not always valid
  _Atomic(ac_u32) count;
  _Atomic(ac_u64) processed;
} AcMpscFifoIn;

#endif
