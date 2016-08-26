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

#include <ac_cache_line.h>
#include <ac_debug_printf.h>
#include <ac_memmgr.h>

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
  mp->msgs_unaligned = AC_NULL;
  mp->msgs = AC_NULL;
  mp->len_data = len_data;
  
  // Allocate and align
  AcU64 size_entry = (sizeof(AcMessage) + len_data + AC_MAX_CACHE_LINE_LEN - 1)
    & ~(AC_MAX_CACHE_LINE_LEN - 1);
  ac_debug_printf("AcMessagePool_init: mp=%p len_data=%d sizeof(AcMessage)=%d size_entry=%d CACHE_LINE_LEN=%d\n",
      mp, len_data, sizeof(AcMessage), size_entry, AC_MAX_CACHE_LINE_LEN);
  AcU64 size = (size_entry * msg_count) + AC_MAX_CACHE_LINE_LEN - 1;
  mp->msgs_unaligned = ac_calloc(1, size);
  if (mp->msgs_unaligned == AC_NULL) {
    return AC_STATUS_OUT_OF_MEMORY;
  }
  mp->msgs = (AcMessage*)(((AcUptr)mp->msgs_unaligned + AC_MAX_CACHE_LINE_LEN - 1)
        & ~(AC_MAX_CACHE_LINE_LEN - 1));
  ac_debug_printf("AcMessagePool_init: mp=%p msgs_unaligned=%p msgs=%p\n",
      mp, mp->msgs_unaligned, mp->msgs);

  // Init
  status = AcMpscRingBuff_init(&mp->rb, msg_count);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Add messages
  void* base = mp->msgs;
  for (AcU32 i = 0; i < msg_count; i++) {
    AcMessage* msg = (AcMessage*)(base + (i * size_entry));
    if (!AcMpscRingBuff_add_mem(&mp->rb, msg)) {
      status = AC_STATUS_ERR;
      goto done;
    }
    msg->mp = mp;
    msg->next.next = AC_NULL;
    msg->next.msg = msg;
  }

done:
  if (status != AC_STATUS_OK && mp != AC_NULL) {
    ac_free(mp->msgs_unaligned);
  } else {
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
  ac_free(mp->msgs_unaligned);
}
