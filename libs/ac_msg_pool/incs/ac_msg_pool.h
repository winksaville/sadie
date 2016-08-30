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
 * An ac_msg_pool contains AcMsg's which can be used for sending
 * information between components. The routine AcMsg_get_msg may only
 * be called from a single thread, but the AcMsg_ret_msg may be called
 * by any thread.
 */

#ifndef SADIE_LIBS_AC_MSG_POOL_INCS_AC_MSG_POOL_H
#define SADIE_LIBS_AC_MSG_POOL_INCS_AC_MSG_POOL_H

#include <ac_inttypes.h>
#include <ac_msg.h>
#include <ac_mpsc_ring_buff.h>
#include <ac_status.h>

typedef struct AcMsgPool {
  AcMpscRingBuff rb;      ///< Ring buffer to hold the messages
  AcU32 len_extra;         ///< Length of the data array in each message
  void* msgs_raw;         ///< If !AC_NULL raw ponter to pass to ac_free
  AcMsg* msgs;            ///< msgs aligned to AC_MAC_CACHE_LINE_LEN
  void* next_ptrs_raw;    ///< if !AC_NULL raw pointer to pass to ac_free
  AcNextPtr* next_ptrs;   ///< next_ptrs for each message
} AcMsgPool;

/**
 * Get a message from a pool
 *
 * @param pool is a previously created pool
 *
 * @return a message or AC_NULL if none available, if !AC_NULL
 * the msg->len_extra will be initialized to len_extra as defined
 * in the call to AcMsgPool_init.
 */
static inline AcMsg* AcMsgPool_get_msg(AcMsgPool* mp) {
  if (mp == AC_NULL) {
    return AC_NULL;
  }
  AcMsg* msg = AcMpscRingBuff_rmv_mem(&mp->rb);
  if (msg != AC_NULL) {
    msg->len_extra = mp->len_extra;
  }
  return msg;
}

/**
 * Ret a message to a pool
 *
 * @param pool is a previously created pool
 * @param msg a message to return the the pool, AC_NULL is ignored
 */
static inline void AcMsgPool_ret_msg(AcMsg* msg) {
  if (msg == AC_NULL || msg->mp == AC_NULL) {
    return;
  }
  AcMpscRingBuff_add_mem(&msg->mp->rb, msg);
}


/**
 * Initialize a message pool
 *
 * @params pool to initialize
 * @params msg_count is number of messages for this pool, must be power of 2 and > 0
 * @params len_extra is number of bytes to allocate for the messages data array, 0 is OK
 *
 * @return 0 (AC_STATUS_OK) if successful
 */
AcStatus AcMsgPool_init(AcMsgPool* pool, AcU32 msg_count, AcU32 len_extra);

/**
 * Deinitialize the message pool
 *
 * @params pool is a pool created by AcMsgPool_alloc
 */
void AcMsgPool_deinit(AcMsgPool* pool);

#endif
