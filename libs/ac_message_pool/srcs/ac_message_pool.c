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

#include <ac_message_pool.h>

#include <ac_assert.h>
#include <ac_cache_line.h>
#include <ac_debug_printf.h>
#include <ac_intmath.h>
#include <ac_memmgr.h>

/**
 * Allocate count elements each at least size long and each aligned
 * on the alignment value. The value in pRaw is the unaligned pointer
 * and is passed to ac_free. The value in pAligned is the address of
 * the first element.
 *
 * @param count is the number of elements to create
 * @param size is the minimum size of each element created
 * @param alignment is a value to align on, such as AC_MAX_CACHE_LINE_LEN
 * @param pRaw is the address to return the raw pointer passed to ac_free
 * @param pAligned is the address to return the aligned pointer of the frist element
 * @param pElemSize is length of each element which will be a multipel of alignment
 */
AcStatus ac_calloc_align(AcU32 count, AcU32 size, AcU32 alignment,
    void** pRaw, void** pAligned, AcU32* pElemSize) {
  ac_debug_printf("ac_calloc_align:+count=%u size=%u alignment=%u\n"
                  "                 pRaw=%p pAligned=%p pElemSize=%p\n",
                  count, size, alignment, pRaw, pAligned, pElemSize);
  AcStatus status;
  void* raw = AC_NULL;
  void* aligned = AC_NULL;
  AcU32 elem_size = 0;
  AcUptr mask = 0;

  if (count == 0 | size == 0 | AC_COUNT_ONE_BITS(alignment) != 1
      | pRaw == AC_NULL | pAligned == AC_NULL | pElemSize == AC_NULL) {
    ac_debug_printf("ac_calloc_align: BAD_PARAM\n");
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  mask = ~((AcUptr)alignment - 1);
  elem_size = (size + alignment - 1) & mask;
  AcU64 allocation_size = (elem_size * count) + alignment - 1;
  raw = ac_calloc(1, allocation_size);
  if (raw == AC_NULL) {
    ac_debug_printf("ac_calloc_align: OUT_OF_MEMORY\n");
    status = AC_STATUS_OUT_OF_MEMORY;
    goto done;
  }
  aligned = (void*)((((AcUptr)raw) + alignment - 1) & mask);

  status = AC_STATUS_OK;

done:
  if (status != AC_STATUS_OK) {
    ac_free(raw);
  } else {
    *pRaw = raw;
    *pAligned = aligned;
    *pElemSize = elem_size;
  }

  ac_debug_printf("ac_calloc_align:-raw=%p aligned=%p elem_size=%u status=%u\n",
      raw, aligned, elem_size, status);
  return status;
}

/**
 * @see ac_message_pool.h
 */
AcStatus AcMessagePool_init(AcMessagePool* mp, AcU32 msg_count, AcU32 len_data) {
  ac_debug_printf("AcMessagePool_init:+mp=%p msg_count=%u len_data=%u\n",
      mp, msg_count, len_data);
  AcStatus status;

  if (mp == AC_NULL) {
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }
  mp->msgs_raw = AC_NULL;
  mp->next_ptrs_raw = AC_NULL;
  mp->msgs = AC_NULL;
  mp->len_data = len_data;
  
  // Allocate and align the messages
  AcU32 size_entry;
  status = ac_calloc_align(msg_count, sizeof(AcMessage) + len_data, AC_MAX_CACHE_LINE_LEN,
      &mp->msgs_raw, (void**)&mp->msgs, &size_entry);
  ac_debug_printf("AcMessagePool_init: mp=%p msgs_raw=%p msgs=%p size_entry=%u status=%u\n",
      mp, mp->msgs_raw, mp->msgs, size_entry, status);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Allocate and align the AcNextPtrs
  // BUG These AcNextPtrs need to be managed globally.
  AcU32 size_next_entry;
  status = ac_calloc_align(msg_count, sizeof(AcNextPtr), AC_MAX_CACHE_LINE_LEN,
      &mp->next_ptrs_raw, (void**)&mp->next_ptrs, &size_next_entry);
  ac_debug_printf("AcMessagePool_init: mp=%p next_ptrs_raw=%p next_ptrs=%p size_next_entry=%u status=%u\n",
      mp, mp->next_ptrs_raw, mp->msgs, size_next_entry, status);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Init the ring buffer
  status = AcMpscRingBuff_init(&mp->rb, msg_count);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Add messages
  void* base = mp->msgs;
  void* base_np = mp->next_ptrs;
  for (AcU32 i = 0; i < msg_count; i++) {
    AcMessage* msg = (AcMessage*)base;
    AcNextPtr* np = (AcNextPtr*)base_np;

    // Init next_ptr fields
    np->next = AC_NULL;
    np->msg = AC_NULL;

    // Init msg fields
    msg->mp = mp;
    msg->next_ptr = np;

    // Add msg to ring buffer
    if (!AcMpscRingBuff_add_mem(&mp->rb, msg)) {
      ac_fail("AcMessagePool_init: WTF should always be able to add msg");
      status = AC_STATUS_ERR;
      goto done;
    }

    // Advance to next entries
    base += size_entry;
    base_np += size_next_entry;
  }

done:
  if (status != AC_STATUS_OK && mp != AC_NULL) {
    ac_free(mp->msgs_raw);
    ac_free(mp->next_ptrs_raw);
  }
  ac_debug_printf("AcMessagePool_init:-mp=%p msg_count=%u len_data=%u status=%d\n",
      mp, msg_count, len_data, status);
  return status;
}

/**
 * @see ac_message_pool.h
 */
void AcMessagePool_deinit(AcMessagePool* mp) {
  if (mp == AC_NULL) {
    return;
  }
  AcMpscRingBuff_deinit(&mp->rb);

  // We ASSUME none of the message are being used!!!
  ac_free(mp->msgs_raw);

  // BUG: We can't free next_ptrs because they could still be in use!!!
  // These can only be freed when all components have stopped. We will
  // need a "global" manager to free them
  //ac_free(mp->next_ptrs_raw);
}
