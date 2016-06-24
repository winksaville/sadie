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
 * The FIFO is composed of a singularly linked list of AcBuff's
 * with a next pointer stored in each element. The list oldest
 * element, which is removed first as this is a FIFO, starts
 * at the fifo->tail->hdr.next and ends with a fifo->head->hdr.next
 * being AC_NULL.
 *
 * A consequence of the algorithm is that when an AcBuff is
 * added to the FIFO a different AcBuff is returned but the
 * contents of AcBuff.data and its size are copied to this
 * new buffer. THEREFORE in general AcBuff's should be small
 * UNLESS AcMpscFifo_rmv_ac_buff_raw is used.
 *
 * If AcMpscFifo_rmv_ac_buff_arw is called no data is copied
 *
 *
 * Some details:
 *
 * A AcMpscFifo has two fields phead and ptail:
 *   typedef struct AcMpscFifo {
 *     AcBuff *head;
 *     AcBuff *tail;
 *   } AcMpscFifo;
 *
 * A AcBuff is declared as:
 *   typedef struct __attribute__((packed)) AcBuffHdr {
 *     AcBuff* next;         // Next AcBuff
 *     AcMpscFifo* pool_fifo;// Poll fifo this AcBuff is allocated from
 *     ac_u32 data_size;     // Size of the data array following this header
 *     ac_u32 user_size;     // Size of the user area in data array
 *   } AcBuffHdr;
 *
 *   typedef struct __attribute__((packed)) AcBuff {
 *     AcBuffHdr hdr;        // The header
 *     ac_u8 data[];         // The buffers data
 *   } AcBuff;
 *
 *
 * When AcMpscFifo_init is called an empty fifo is created
 * with one stub AcBuff whose hdr.next field is AC_NULL and
 * AcMpscFifo.head and .tail both point at this stub.
 *
 * The fifo->head is where AcBuff's are added to the queue and
 * points to the most recent element and is at the end of
 * the list thus this element always has its next == AC_NULL.
 *
 * fifo->tail->hdr.next is the oldest AcBuff and is the next
 * element that will be removed and is only AC_NULL when the
 * FIFO is empty.
 *
 *
 * Add an AcBuff to the FIFO, invoked by multiple threads
 * (Multiple Producers):
 *
 *    // Step 1) Set buff->next to AC_NULL as this is the end
 *    // of the list.
 *    buff->hdr.next = AC_NULL;
 *
 *    // Step 2) Exchange fifo->head and buff so fifo->head now
 *    // points at the new buff. Serializes with other producers
 *    // calling this routine.
 *    AcBuff *prev_head = __atomic_exchange_n(&fifo->head, buff, __ATOMIC_ACQ_REL);
 *
 *    // Step 3) Store buff into the next of the previous head
 *    // which actually adds the new buff to the fifo. Serialize
 *    // with rmv_ac_buff Step 4 if the list is empty.
 *    __atomic_store_n(&prev_head->hdr.next, buff, __ATOMIC_RELEASE);
 *
 * Remove an AcBuff from the FIFO, invoked by one thread
 * (Single Consumer):
 *
 *    // Step 1) Use the current stub value to return the result
 *    AcBuff* result = fifo->tail;
 *
 *    // Step 2) Get the oldest element, serialize with Step 3 in add ac_buff
 *    AcBuff* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);
 *
 *    // Step 3) If list is empty return AC_NULL
 *    if (oldest == AC_NULL) {
 *      return AC_NULL;
 *    }
 *
 *    // Step 4) The oldest becomes new tail stub. If we are removing
 *    // the last element then poldest->pnext is AC_NULL because add ac_buff
 *    // made it so.
 *    fifo->tail = oldest;
 *
 *    // Step 5) Copy the contents of oldest AcBuff to result
 *    result->hdr.user_size = oldest->hdr.user_size;
 *    ac_memcpy(result->data, oldest->data, oldest->hdr.user_size);
 *
 *    // Step 6) Return result and set next to AC_NULL
 *    result->hdr.next = AC_NULL;
 *    return result;
 *
 * Raw remove an AcBuff from the FIFO, invoked by one thread
 * (Single Consumer):
 *
 *    // Step 1) Use the current stub value to return the result
 *    AcBuff *result = fifo->tail;
 *
 *    // Step 2) Get the oldest element, serialize with Step 3 in add_buff
 *    AcBuff* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);
 *
 *    // Step 3) If list is empty return AC_NULL
 *    if (oldest == AC_NULL) {
 *      return AC_NULL;
 *    }
 *
 *    // Step 4) The oldest becomes new tail stub. If we are removing
 *    // the last element then oldest->hdr.next is AC_NULL because add_ac_buff
 *    // made it so.
 *    fifo->tail = oldest;
 *
 *    // Step 5) Return result and we'll set next to AC_NULL
 *    result->hdr.next = AC_NULL;
 *    return result;
 */

#include <ac_mpsc_fifo.h>
#include <ac_mpsc_fifo_dbg.h>

#include <ac_assert.h>
#include <ac_buff_dbg.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_memcpy.h>

/**
 * @see ac_mpsc_fifo.h
 */
void AcMpscFifo_add_ac_buff(AcMpscFifo* fifo, AcBuff* buff) {
  ac_assert(fifo != AC_NULL);

  if (buff != AC_NULL) {
    // Assert buff is compatible
    // TODO, maybe have routine return AcStatus and return AC_STATUS_BAD_PARAM??
    ac_assert(fifo->tail->hdr.data_size == buff->hdr.data_size);

    // Step 1) Set buff->next to AC_NULL as this is the end
    // of the list.
    buff->hdr.next = AC_NULL;

    // Step 2) Exchange fifo->head and buff so fifo->head now
    // points at the new buff. Serializes with other producers
    // calling this routine.
    AcBuff *prev_head = __atomic_exchange_n(&fifo->head, buff, __ATOMIC_ACQ_REL);

    // Step 3) Store buff into the next of the previous head
    // which actually adds the new buff to the fifo. Serialize
    // with rmv_ac_buff Step 4 if the list is empty.
    __atomic_store_n(&prev_head->hdr.next, buff, __ATOMIC_RELEASE);
  }
}

/**
 * @see ac_mpsc_fifo.h
 */
AcBuff* AcMpscFifo_rmv_ac_buff(AcMpscFifo* fifo) {
  ac_assert(fifo != AC_NULL);

  // Step 1) Use the current stub value to return the result
  AcBuff* result = fifo->tail;

  // Step 2) Get the oldest element, serialize with Step 3 in add ac_buff
  AcBuff* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);

  // Step 3) If list is empty return AC_NULL
  if (oldest == AC_NULL) {
    return AC_NULL;
  }

  // Step 4) The oldest becomes new tail stub. If we are removing
  // the last element then poldest->pnext is AC_NULL because add ac_buff
  // made it so.
  fifo->tail = oldest;

  // Step 5) Copy the contents of oldest AcBuff to result
  result->hdr.user_size = oldest->hdr.user_size;
  ac_memcpy(result->data, oldest->data, oldest->hdr.user_size);

  // Step 6) Return result and we'll set next to AC_NULL
  result->hdr.next = AC_NULL;
  return result;
}

/**
 * @see ac_mpsc_fifo.h
 */
AcBuff* AcMpscFifo_rmv_ac_buff_raw(AcMpscFifo* fifo) {
  if (fifo == AC_NULL) {
    return AC_NULL;
  }

  // Step 1) Use the current stub value to return the result
  AcBuff *result = fifo->tail;

  // Step 2) Get the oldest element, serialize with Step 3 in add_buff
  AcBuff* oldest = __atomic_load_n(&result->hdr.next, __ATOMIC_ACQUIRE);

  // Step 3) If list is empty return AC_NULL
  if (oldest == AC_NULL) {
    return AC_NULL;
  }

  // Step 4) The oldest becomes new tail stub. If we are removing
  // the last element then oldest->hdr.next is AC_NULL because add_ac_buff
  // made it so.
  fifo->tail = oldest;

  // Step 5) Return result and we'll set next to AC_NULL
  result->hdr.next = AC_NULL;
  return result;
}

/**
 * @see ac_mpsc_fifo.h
 */
void AcMpscFifo_deinit(AcMpscFifo* fifo) {
  // Assert that the fifo is empty
  ac_assert(fifo != AC_NULL);
  ac_assert(fifo->head != AC_NULL);
  ac_assert(fifo->tail != AC_NULL);
  ac_assert(fifo->tail->hdr.next == AC_NULL);
  ac_assert(fifo->tail == fifo->head);

  // Return the stub to its fifo and null head and tail
  AcBuff* stub = fifo->head;
  fifo->head = AC_NULL;
  fifo->tail = AC_NULL;

  if (stub->hdr.pool_fifo != fifo) {
    AcBuff_ret(stub);
  }
}

/**
 * @see ac_mpsc_fifo.h
 */
AcStatus AcMpscFifo_init_with_stub(AcMpscFifo* fifo, AcBuff* stub) {
  int status;

  if ((fifo == AC_NULL) || (stub == AC_NULL)) {
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  stub->hdr.next = AC_NULL;
  fifo->head = stub;
  fifo->tail = stub;

  status = AC_STATUS_OK;

done:
  return (AcStatus)status;
}

/**
 * @see ac_mpsc_fifo.h
 */
AcStatus AcMpscFifo_init(AcMpscFifo* fifo, ac_u32 data_size) {
  AcStatus status;

  AcBuff* stub;
  if (AcBuff_alloc(AC_NULL, 1, data_size, 0, &stub) != AC_STATUS_OK) {
    status = AC_STATUS_OUT_OF_MEMORY;
    goto done;
  }

  status = AcMpscFifo_init_with_stub(fifo, stub);

done:
  return status;
}

/**
 * Early initialize module (deprecated????)
 */
__attribute__((constructor))
void AcMpscFifo_early_init(void) {
}
