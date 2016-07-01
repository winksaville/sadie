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
 * A AcMpscFifo is a wait free/thread safe multi-producer
 * single consumer first in first out queue. This algorithm
 * is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 *
 * The FIFO has a head and tail, the elements are added
 * to the head of the queue and removed from the tail. To
 * allow for a wait free algorithm a stub element is used
 * so that a single atomic instruction can be used to add and
 * remove an element.
 *
 * The FIFO is composed of a singularly linked list of AcMem's
 * with a next pointer stored in each element. The list oldest
 * element, which is removed first as this is a FIFO, starts
 * at the fifo->tail->hdr.next and ends with a fifo->head->hdr.next
 * being AC_NULL.
 *
 * A consequence of the algorithm is that when an AcMem is
 * added to the FIFO a different AcMem is returned but the
 * contents of AcMem.data and its size are copied to this
 * new memer. THEREFORE in general AcMem's should be small
 * UNLESS AcMpscFifo_rmv_ac_mem_raw is used.
 *
 * If AcMpscFifo_rmv_ac_mem_arw is called no data is copied
 *
 *
 * Some details:
 *
 * A AcMpscFifo has two fields phead and ptail:
 *   typedef struct AcMpscFifo {
 *     AcMem *head;
 *     AcMem *tail;
 *   } AcMpscFifo;
 *
 * A AcMem is declared as:
 *   typedef struct __attribute__((packed)) AcMemHdr {
 *     AcMem* next;         // Next AcMem
 *     AcMpscFifo* pool_fifo;// Poll fifo this AcMem is allocated from
 *     ac_u32 data_size;     // Size of the data array following this header
 *     ac_u32 user_size;     // Size of the user area in data array
 *   } AcMemHdr;
 *
 *   typedef struct __attribute__((packed)) AcMem {
 *     AcMemHdr hdr;        // The header
 *     ac_u8 data[];         // The memers data
 *   } AcMem;
 *
 *
 * When AcMpscFifo_init is called an empty fifo is created
 * with one stub AcMem whose hdr.next field is AC_NULL and
 * AcMpscFifo.head and .tail both point at this stub.
 *
 * The fifo->head is where AcMem's are added to the queue and
 * points to the most recent element and is at the end of
 * the list thus this element always has its next == AC_NULL.
 *
 * fifo->tail->hdr.next is the oldest AcMem and is the next
 * element that will be removed and is only AC_NULL when the
 * FIFO is empty.
 *
 *
 * Add an AcMem to the FIFO, invoked by multiple threads
 * (Multiple Producers):
 *
 *    // Step 1) Set mem->next to AC_NULL as this is the end
 *    // of the list.
 *    mem->hdr.next = AC_NULL;
 *
 *    // Step 2) Exchange fifo->head and mem so fifo->head now
 *    // points at the new mem. Serializes with other producers
 *    // calling this routine.
 *    AcMem *prev_head = __atomic_exchange_n(&fifo->head, mem, __ATOMIC_ACQ_REL);
 *
 *    // Step 3) Store mem into the next of the previous head
 *    // which actually adds the new mem to the fifo. Serialize
 *    // with rmv_ac_mem Step 4 if the list is empty.
 *    __atomic_store_n(&prev_head->hdr.next, mem, __ATOMIC_RELEASE);
 *
 * Remove an AcMem from the FIFO, invoked by one thread
 * (Single Consumer):
 *
 *    // Step 1) Use the current stub value to return the result
 *    AcMem* result = fifo->tail;
 *
 *    // Step 2) Get the oldest element, serialize with Step 3 in add ac_mem
 *    AcMem* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);
 *
 *    // Step 3) If list is empty return AC_NULL
 *    if (oldest == AC_NULL) {
 *      return AC_NULL;
 *    }
 *
 *    // Step 4) The oldest becomes new tail stub. If we are removing
 *    // the last element then poldest->pnext is AC_NULL because add ac_mem
 *    // made it so.
 *    fifo->tail = oldest;
 *
 *    // Step 5) Copy the contents of oldest AcMem to result
 *    result->hdr.user_size = oldest->hdr.user_size;
 *    ac_memcpy(result->data, oldest->data, oldest->hdr.user_size);
 *
 *    // Step 6) Return result and set next to AC_NULL
 *    result->hdr.next = AC_NULL;
 *    return result;
 *
 * Raw remove an AcMem from the FIFO, invoked by one thread
 * (Single Consumer):
 *
 *    // Step 1) Use the current stub value to return the result
 *    AcMem *result = fifo->tail;
 *
 *    // Step 2) Get the oldest element, serialize with Step 3 in add_mem
 *    AcMem* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);
 *
 *    // Step 3) If list is empty return AC_NULL
 *    if (oldest == AC_NULL) {
 *      return AC_NULL;
 *    }
 *
 *    // Step 4) The oldest becomes new tail stub. If we are removing
 *    // the last element then oldest->hdr.next is AC_NULL because add_ac_mem
 *    // made it so.
 *    fifo->tail = oldest;
 *
 *    // Step 5) Return result and we'll set next to AC_NULL
 *    result->hdr.next = AC_NULL;
 *    return result;
 */

#define NDEBUG

#include <ac_mpsc_fifo.h>
#include <ac_mpsc_fifo_dbg.h>

#include <ac_assert.h>
#include <ac_mem_dbg.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_memcpy.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>

/**
 * @see ac_mpsc_fifo.h
 */
void AcMpscFifo_add_ac_mem(AcMpscFifo* fifo, AcMem* mem) {
  ac_assert(fifo != AC_NULL);

  ac_debug_printf("AcMpscFifo_add_ac_mem:+fifo=%p mem=%p\n",
      fifo, mem);

  if (mem != AC_NULL) {
    ac_debug_printf("AcMpscFifo_add_ac_mem: fifo=%p mem=%p tail=%p tail.data_size=%d mem.data_size=%d\n",
        fifo, mem, fifo->tail, fifo->tail->hdr.data_size, mem->hdr.data_size);

    // Assert mem is compatible
    // TODO, maybe have routine return AcStatus and return AC_STATUS_BAD_PARAM??
#if 1
    ac_u8 b3 = ((ac_u8*)&fifo->tail)[3];
    if (((fifo->tail != AC_NULL) && (((ac_uptr)(fifo->tail) & (ac_uptr)0xFFFFFF) == 0)) || (b3 == 0)) {
      ac_printf("Crash add_ac_mem: fifo=%p\n", fifo);
      ac_printf("Crash add_ac_mem: fifo->head=%p\n", fifo->head);
      ac_printf("Crash add_ac_mem: fifo->tail=%p\n", fifo->tail);
      ac_printf("b3=0x%x\n", b3);
      AcMem* n = fifo->tail->hdr.next;
      ac_printf("n=%p\n", n);
      ac_assert((fifo->tail == AC_NULL) || (((ac_uptr)(fifo->tail) & (ac_uptr)0xFFFFFF) != 0) || (b3 != 0));
    }
#else
    ac_assert(fifo->tail->hdr.data_size == mem->hdr.data_size);
#endif

    // Step 1) Set mem->next to AC_NULL as this is the end
    // of the list.
    mem->hdr.next = AC_NULL;

    // Step 2) Exchange fifo->head and mem so fifo->head now
    // points at the new mem. Serializes with other producers
    // calling this routine.
    AcMem *prev_head = __atomic_exchange_n(&fifo->head, mem, __ATOMIC_ACQ_REL);

    // Step 3) Store mem into the next of the previous head
    // which actually adds the new mem to the fifo. Serialize
    // with rmv_ac_mem Step 4 if the list is empty.
    __atomic_store_n(&prev_head->hdr.next, mem, __ATOMIC_RELEASE);
  }

  ac_debug_printf("AcMpscFifo_add_ac_mem:-fifo=%p mem=%p\n",
      fifo, mem);
}

/**
 * @see ac_mpsc_fifo.h
 */
AcMem* AcMpscFifo_rmv_ac_mem(AcMpscFifo* fifo) {
  ac_assert(fifo != AC_NULL);

  // Step 1) Use the current stub value to return the result
  AcMem* result = fifo->tail;

  // Step 2) Get the oldest element, serialize with Step 3 in add ac_mem
  AcMem* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);

  // Step 3) If list is empty return AC_NULL
  if (oldest == AC_NULL) {
    return AC_NULL;
  }

  // Step 4) The oldest becomes new tail stub. If we are removing
  // the last element then poldest->pnext is AC_NULL because add ac_mem
  // made it so.
  fifo->tail = oldest;

  // Step 5) Copy the contents of oldest AcMem to result
  result->hdr.user_size = oldest->hdr.user_size;
  ac_memcpy(result->data, oldest->data, oldest->hdr.user_size);

  // Step 6) Return result and we'll set next to AC_NULL
  result->hdr.next = AC_NULL;
  return result;
}

/**
 * @see ac_mpsc_fifo.h
 */
AcMem* AcMpscFifo_rmv_ac_mem_raw(AcMpscFifo* fifo) {
  if (fifo == AC_NULL) {
    return AC_NULL;
  }

  // Step 1) Use the current stub value to return the result
  AcMem *result = fifo->tail;

  ac_u8 b3 = ((ac_u8*)&result)[3];
  if (((result != AC_NULL) && (((ac_uptr)(result) & (ac_uptr)0xFFFFFF) == 0)) || (b3 == 0)) {
    ac_printf("Crash rmv_ac_mem: result=%p\n", result);
    ac_printf("Crash rmv_ac_mem: fifo=%p\n", fifo);
    ac_printf("Crash rmv_ac_mem: fifo->head=%p\n", fifo->head);
    ac_printf("Crash rmv_ac_mem: fifo->tail=%p\n", fifo->tail);
    ac_printf("b3=0x%x\n", b3);
    AcMem* n = result->hdr.next;
    ac_printf("n=%p\n", n);
    ac_assert(((result == AC_NULL) || ((ac_uptr)(result) & (ac_uptr)0xFFFFFF) != 0) || (b3 != 0));
  }

  // Step 2) Get the oldest element, serialize with Step 3 in add_mem
  AcMem* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);

  // Step 3) If list is empty return AC_NULL
  if (oldest == AC_NULL) {
    return AC_NULL;
  }

  if (((oldest != AC_NULL) && (((ac_uptr)(oldest) & (ac_uptr)0xFFFFFF) == 0)) || (b3 == 0)) {
    ac_printf("Crash rmv_ac_mem: oldest=%p\n", oldest);
    ac_printf("Crash rmv_ac_mem: fifo=%p\n", fifo);
    ac_printf("Crash rmv_ac_mem: fifo->head=%p\n", fifo->head);
    ac_printf("Crash rmv_ac_mem: fifo->tail=%p\n", fifo->tail);
    ac_printf("b3=0x%x\n", b3);
    AcMem* n = oldest->hdr.next;
    ac_printf("n=%p\n", n);
    ac_assert((oldest == AC_NULL) || (((ac_uptr)(oldest) & (ac_uptr)0xFFFFFF) != 0) || (b3 != 0));
  }

  // Step 4) The oldest becomes new tail stub. If we are removing
  // the last element then oldest->hdr.next is AC_NULL because add_ac_mem
  // made it so.
  fifo->tail = oldest;

  if (((fifo->tail != AC_NULL) && (((ac_uptr)(fifo->tail) & (ac_uptr)0xFFFFFF) == 0)) || (b3 == 0)) {
    ac_printf("Crash rmv_ac_mem: fifo=%p\n", fifo);
    ac_printf("Crash rmv_ac_mem: fifo->head=%p\n", fifo->head);
    ac_printf("Crash rmv_ac_mem: fifo->tail=%p\n", fifo->tail);
    ac_printf("b3=0x%x\n", b3);
    AcMem* n = fifo->tail->hdr.next;
    ac_printf("n=%p\n", n);
    ac_assert((fifo->tail == AC_NULL) || (((ac_uptr)(fifo->tail) & (ac_uptr)0xFFFFFF) != 0) || (b3 != 0));
  }

  // Step 5) Return result and we'll set next to AC_NULL
  result->hdr.next = AC_NULL;
  return result;
}

/**
 * @see ac_mpsc_fifo.h
 */
void AcMpscFifo_deinit(AcMpscFifo* fifo) {
  ac_debug_printf("AcMpscFifo_deinit:+fifo=%p\n", fifo);

  // Assert fifo is "valid" and it owns only one AcMem
  ac_assert(fifo != AC_NULL);
  ac_assert(fifo->count == 1);
  ac_assert(fifo->mem_array != AC_NULL);
  ac_assert(fifo->head != AC_NULL);
  ac_assert(fifo->tail != AC_NULL);

  // Assert that the fifo is empty
  ac_assert(fifo->tail->hdr.next == AC_NULL);
  ac_assert(fifo->tail == fifo->head);

  // Return the stub to its fifo and null head and tail
  AcMem* stub = fifo->head;
  fifo->head = AC_NULL;
  fifo->tail = AC_NULL;

  if (stub->hdr.pool_fifo != fifo) {
    ac_debug_printf("AcMpscFifo_deinit: fifo=%p return non-owned stub=%p stub->pool_fifo=%p\n",
        fifo, stub, stub->hdr.pool_fifo);
    AcMem_ret(stub);
  } else {
    ac_debug_printf("AcMpscFifo_deinit: fifo=%p return owned stub=%p stub->pool_fifo=%p\n",
        fifo, stub, stub->hdr.pool_fifo);
    AcMem_free(stub);
  }

  ac_debug_printf("AcMpscFifo_deinit:-fifo=%p\n", fifo);
}

/**
 * @see ac_mpsc_fifo.h
 */
void AcMpscFifo_deinit_full(AcMpscFifo* fifo) {
  ac_debug_printf("AcMpscFifo_deinit:+fifo=%p count=%d\n", fifo, fifo->count);

  // Assert fifo is "valid" and it owns more than one AcMem
  ac_assert(fifo != AC_NULL);
  ac_assert(fifo->count > 1);
  ac_assert(fifo->mem_array != AC_NULL);
  ac_assert(fifo->head != AC_NULL);
  ac_assert(fifo->tail != AC_NULL);

  // Loop through the fifo removing AcMem's verifying all they
  // all belong to this fifo.
  for (ac_u32 i = 0; i < fifo->count - 1; i++) {
    AcMem* mem = AcMpscFifo_rmv_ac_mem(fifo);
    AcMem_debug_print("AcMpscFifo_deinit: mem=", mem);

    // Be sure mem is valid and it belongs to this fifo
    ac_assert(mem != AC_NULL);
    ac_assert(mem->hdr.pool_fifo == fifo);
  }

  // Remove the stub
  AcMem* stub = fifo->head;
  fifo->head = AC_NULL;
  fifo->tail = AC_NULL;

  // Verify it belongs to this fifo
  ac_assert(stub != AC_NULL);
  ac_assert(stub->hdr.pool_fifo == fifo);

  ac_debug_printf("AcMpscFifo_deinit: fifo=%p ret mem_array=%p\n", fifo, fifo->mem_array);
  AcMem_free(fifo->mem_array);
  ac_debug_printf("AcMpscFifo_deinit:-fifo=%p\n", fifo);
}

/**
 * @see ac_mpsc_fifo.h
 */
AcStatus AcMpscFifo_init_and_alloc(AcMpscFifo* fifo, ac_u32 count,
    ac_u32 data_size) {
  AcStatus status;

  ac_debug_printf("AcMpscFifo_init_and_alloc:+fifo=%p count=%d data_size=%d\n",
      fifo, count, data_size);

  if (fifo == AC_NULL) {
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  // Allocate one extra for the stub
  fifo->count = count + 1;

  // Allocate the mem_array for the fifo including an extra one for
  // the stub, AC_NULL is passed for fifo because the fifo isn't
  // initialized yet and thus AcMem_alloc can't automatically add it.
  //
  //
  // TODO: Maybe AcMem_alloc should not have a count (or we only use 1)
  // and when we deinit we free them in a loop one at a time, that way
  // additional memory blocks could be added to the fifo for management
  // at any time.
  status = AcMem_alloc(AC_NULL, fifo->count, data_size, 0, &fifo->mem_array);
  ac_debug_printf("AcMpscFifo_init_and_alloc: fifo=%p status=%d mem_array=%p\n",
      fifo, status, fifo->mem_array);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Add the rest of the AcMem's to the fifo
  for (ac_u32 i = 0; i < fifo->count; i++) {
    AcMem* mem = AcMem_get_nth(fifo->mem_array, i);
    mem->hdr.pool_fifo = fifo;
    if (i == 0) {
      // Add the stub
      ac_debug_printf("AcMpscFifo_init_and_alloc: fifo=%p add stub=%p\n",
          fifo, mem);
      mem->hdr.next = AC_NULL;
      fifo->head = mem;
      fifo->tail = mem;
    } else {
      // Add non-stub
      AcMpscFifo_add_ac_mem(fifo, mem);
    }
  }

done:
  if (status != AC_STATUS_OK) {
    if (fifo != AC_NULL) {
      AcMem_free(fifo->mem_array);
    }
  }

  ac_debug_printf("AcMpscFifo_init_and_alloc:-fifo=%p status=%d\n",
      fifo, status);
  return status;
}

/**
 * @see ac_mpsc_fifo.h
 */
AcStatus AcMpscFifo_init(AcMpscFifo* fifo, ac_u32 data_size) {
  return AcMpscFifo_init_and_alloc(fifo, 0, data_size);
}

/**
 * Early initialize module (deprecated????)
 */
__attribute__((constructor))
void AcMpscFifo_early_init(void) {
}
