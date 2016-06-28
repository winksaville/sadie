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
 * Header for internal use only
 */

#ifndef SADIE_LIBS_AC_MEM_POOL_INCS_AC_MEM_POOL_INTERNAL_H
#define SADIE_LIBS_AC_MEM_POOL_INCS_AC_MEM_POOL_INTERNAL_H

#include <ac_mpsc_fifo.h>

#include <ac_inttypes.h>

// Pool mem pool where each mem->arg1 points to a AcMem
typedef struct AcMemFifo {
  AcMemCountSize mcs;   // Count of mem_array
  AcMem* mem_array;     // The mem_array allocated owned by this AcMemFifo
  AcMpscFifo fifo;      // A fifo of AcMem's available for use
} AcMemFifo;

// Array of AcMemPool one for each different AcMem size
typedef struct AcMemPool {
  ac_u32 count;                 // Number of elements in mem_fifo_array
  AcMemFifo mem_fifo_array[];   // Array of AcMemFifo's
} AcMemPool;


#endif
