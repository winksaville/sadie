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

#include <ac_assert.h>
#include <ac_memmgr.h>
#include <ac_mpscfifo.h>
#include <ac_msg.h>
#include <ac_debug_printf.h>

typedef void (*AcMsgRet)(AcMsgPool mp, AcMsg* msg);

typedef struct msg_pool {
  AcMsgRet AcMsg_ret;
  ac_mpscfifo mpscfifo;
} *AcMsgPool;

/**
 * Create a msg pool
 *
 * @return AC_NULL if no pool
 */
AcMsgPool AcMsgPool_create(ac_u32 count) {
  AcMsgPool pool;

  if (count == 0) {
    pool = AC_NULL;
  } else {
    // Allocate the AcMsgPool
    pool = ac_malloc(sizeof(AcMsgPool));
    ac_debug_printf("AcMsgPool_create: pool=%p\n", pool);
    ac_assert(pool != AC_NULL);

    // Allocate the AcMsg's count + 1 which is the stub
    AcMsg* arena = ac_malloc(sizeof(AcMsg) * (count + 1));
    ac_debug_printf("AcMsgPool_create: arena=%p\n", arena);

    // Initialize the mpscfifo and guarantee pool = what's returned
    ac_assert(ac_mpscfifo_init(&pool->mpscfifo, arena) == &pool->mpscfifo);

    // Add the other messages to the pool
    for (ac_u32 i = 1; i <= count; i++) {
      ac_mpscfifo_add_msg(&pool->mpscfifo, &arena[i]);
      arena[i].pool = pool;
      ac_debug_printf("AcMsgPool_create: adding &arena[%d]=%p pool=%p\n", i, &arena[i], arena[i].pool);
    }
  }

  return pool;
}

/**
 * Get a message from a pool
 *
 * @param pool is a previously created pool
 *
 * @return a message or AC_NULL if none available
 */
AcMsg* AcMsg_get(AcMsgPool mp) {
  AcMsg* msg;

  if (mp == AC_NULL) {
    msg = AC_NULL;
    ac_debug_printf("AcMsg_get: mp is AC_NULL msg=%p\n", msg);
  } else {
    msg = ac_mpscfifo_rmv_msg_raw(&mp->mpscfifo);
    ac_debug_printf("AcMsg_get: msg=%p\n", msg);
  }
  return msg;
}

/**
 * Ret a message to a pool
 *
 * @param msg a message to return the the pool, AC_NULL is ignored
 */
void AcMsg_ret(AcMsg* msg) {
  ac_debug_printf("AcMsg_ret: msg=%p\n", msg);
  if ((msg != AC_NULL) && (msg->pool != AC_NULL)) {
    ac_mpscfifo_add_msg(msg->pool, msg);  
  }
}
