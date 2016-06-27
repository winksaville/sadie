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

#ifndef SADIE_LIBS_AC_MEM_INCS_AC_MEM_H
#define SADIE_LIBS_AC_MEM_INCS_AC_MEM_H

#include <ac_inttypes.h>
//#include <ac_mpsc_fifo.h>
#include <ac_status.h>

typedef struct AcMem AcMem;

typedef struct AcMpscFifo AcMpscFifo;
extern void AcMpscFifo_add_ac_mem(AcMpscFifo* fifo, AcMem* mem);

typedef struct __attribute__((packed)) AcMemHdr {
  AcMem* next;         // Next AcMem
  AcMpscFifo* pool_fifo;// Poll fifo this AcMem is allocated from
  ac_u32 data_size;     // Size of the data array following this header
  ac_u32 user_size;     // Size of the user area in data array
} AcMemHdr;

typedef struct __attribute__((packed)) AcMem {
  AcMemHdr hdr;        // The header
  ac_u8 data[];         // The buffers data
} AcMem;

/**
 * Return the nth AcMem in the array
 *
 * @params array is the pointer to the first element
 * @params index is the index into the array
 *
 * @returns pointer to the element
 */
// TODO: Consider having AcMem_alloc return an array of pointers.
static inline AcMem* AcMem_get_nth(AcMem* array, ac_u32 index) {
    ac_u8* addr = ((ac_u8*)array) + (index  * (sizeof(AcMem) + array->hdr.data_size));
    return (AcMem*)addr;
}

/**
 * Return memory to its pool
 *
 * @params mem is an AcMem
 */
void AcMem_ret(AcMem* mem);

/**
 * Free the mem_array
 *
 * @params mem_array is a set of AcMem's previouslly allocated with
 * AcMem_alloc.
 */
void AcMem_free(AcMem* mem);

/**
 * Allocate one or more AcMem contigiously
 *
 * @params fifo is the fifo this AcMem belongs to
 * @params count is number of AcMem's to allocate
 * @params data_size is size of each memory block
 * @params user_size is size available to user
 * @params ptr_mem_array an out parameter that on success
 * will point to the allocate array of AcMem's.
 *
 * @return 0 (AC_STATUS_OK) if successful and if count > 0 then mem != AC_NULL
 */
// TODO: Consider having AcMem_alloc return an array of pointers
AcStatus AcMem_alloc(AcMpscFifo* fifo, ac_u32 count, ac_u32 data_size,
    ac_u32 user_size, AcMem** ptr_mem_array);

#endif
