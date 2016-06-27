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

#include <ac_mem.h>

#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_status.h>

/**
 * Return a memory to its pool
 *
 * @params mem is an AcMem
 */
void AcMem_ret(AcMem* mem) {
  if (mem != AC_NULL) {
    if (mem->hdr.pool_fifo != AC_NULL) {
      AcMpscFifo_add_ac_mem(mem->hdr.pool_fifo, mem);
    } else {
      AcMem_free(mem);
    }
  }
}

/**
 * Free the mem_array
 *
 * @params mem_array is a set of AcMem's previouslly allocated with
 * AcMem_alloc.
 */
void AcMem_free(AcMem* mem_array) {
  ac_free(mem_array);
}

/**
 * Allocate one or more AcMem contigiously and if fifo is not AC_NULL
 * the AcMem is added to the fifo
 *
 * @params fifo owning this AcMem's allocated or AC_NULL if none.
 * @params count is number of AcMem's to allocate
 * @params data_size is size of each memory block
 * @params user_size is size available to user
 * @params ptr_mem_array an out parameter that on success
 * will point to the allocate array of AcMem's.
 *
 * @return 0 (AC_STATUS_OK) if successful the allocated AcMem's will
 * be initialized and if fifo parameter is not AC_NULL they will be
 * added to the fifo.
 */
AcStatus AcMem_alloc(AcMpscFifo* fifo, ac_u32 count,
    ac_u32 data_size, ac_u32 user_size, AcMem** ptr_mem_array) {
  AcStatus status;
  AcMem* mem_array;

  // Check params
  if ((count == 0) || (data_size == 0) || (data_size < user_size)) {
    status = AC_STATUS_BAD_PARAM;
    mem_array = AC_NULL;
    goto done;
  }

  // Allocate the array and clear it
  mem_array = ac_calloc(count, sizeof(AcMem) + data_size);
  if (mem_array == AC_NULL) {
    status = AC_STATUS_OUT_OF_MEMORY;
    goto done;
  }

  // Initialize headers
  for (ac_u32 i = 0; i < count; i++) {
    AcMem* mem = AcMem_get_nth(mem_array, i);
    mem->hdr.next = AC_NULL;
    mem->hdr.pool_fifo = fifo;
    mem->hdr.data_size = data_size;
    mem->hdr.user_size = user_size;
    if (mem->hdr.pool_fifo != AC_NULL) {
      // TODO: Maybe we shouldn't ever do this because we now have AcMpsc_init_and_alloc
      // also using AcMem_alloc multiple times will screw up freeing because right now
      // we're assume all mems are allocated at once.
      AcMem_ret(mem);
    }
  }

  status = AC_STATUS_OK;

done:
  if (status != AC_STATUS_OK) {
    ac_free(mem_array);
  }

  *ptr_mem_array = mem_array;

  return status;
}
