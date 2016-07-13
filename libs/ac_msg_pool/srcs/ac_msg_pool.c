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

#include <ac_msg_pool.h>

#include <ac_mem_pool.h>
#include <ac_msg.h>
#include <ac_status.h>

/**
 * See ac_msg_pool.h
 */
AcStatus AcMsgPool_get_msg(AcMsgPool* mp, AcMsg** ptr_AcMsg) {
  return AcMemPool_get_mem(mp->mem_pool, sizeof(AcMsg), (void**)ptr_AcMsg);
}

/**
 * See ac_msg_pool.h
 */
void AcMsgPool_ret_msg(AcMsg* msg) {
  AcMemPool_ret_mem(msg);
}

/**
 * See ac_msg_pool.h
 */
AcStatus AcMsgPool_init(AcMsgPool* pool, ac_u32 msg_count) {
  AcStatus status;
  AcMemPoolCountSize mpcs;

  mpcs.count = msg_count;
  mpcs.data_size = sizeof(AcMsg);
  status = AcMemPool_alloc(1, &mpcs, &pool->mem_pool);
  return status;
}

/**
 * See ac_msg_pool.h
 */
void AcMsgPool_deinit(AcMsgPool* pool) {
  AcMemPool_free(pool->mem_pool);
}
