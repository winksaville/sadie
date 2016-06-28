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
 * Debug code for mpscfifo
 */

#ifndef SADIE_LIBS_AC_MEM_POOL_AC_MEM_POOL_DBG_H
#define SADIE_LIBS_AC_MEM_POOL_AC_MEM_POOL_DBG_H

#include <ac_mpsc_fifo.h>

typedef struct AcMemFifo AcMemFifo;

/**
 * Print a AcMemFifo
 */
void AcMemFifo_print(const char* leader, AcMemFifo* mem_fifo);

#ifdef NDEBUG
  #define AcMemPool_debug_print_mem_fifo(leader, pool) ((void)(0))
#else
  #define AcMemPool_debug_print_mem_fifo(leader, pool) AcMemPool_print_mem_fifo(leader, pool)
#endif

/**
 * Print a AcMemPool
 */
void AcMemPool_print(const char* leader, AcMemPool* pool);

#ifdef NDEBUG
  #define AcMemPool_debug_print(leader, pool) ((void)(0))
#else
  #define AcMemPool_debug_print(leader, pool) AcMemPool_print(leader, pool)
#endif

#endif
