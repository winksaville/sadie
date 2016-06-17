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

typedef struct AcMsgPool {
  ac_mpscfifo mpscfifo;
} AcMsgPool;

// We are friends with ac_mpscfifo so using the special init routine
void ac_mpscfifo_init_with_stub(ac_mpscfifo* pq, AcMsg* pstub);

/**
 * Allocate a AcMsgPool
 */
AcMsgPool* AcMsgPool_alloc(void) {
  return ac_malloc(sizeof(AcMsgPool));
}

/**
 * Allocate a AcMsg
 */
AcMsg* AcMsg_alloc(void) {
  return ac_malloc(sizeof(AcMsg));
}

/**
 * Get a message from a pool
 *
 * @param pool is a previously created pool
 *
 * @return a message or AC_NULL if none available
 */
AcMsg* AcMsg_get(AcMsgPool* mp) {
  AcMsg* msg;

  if (mp == AC_NULL) {
    msg = AC_NULL;
    ac_debug_printf("AcMsg_get:-mp is AC_NULL msg=%p\n", msg);
  } else {
    ac_debug_printf("AcMsg_get:+mp=%p\n", mp);
    msg = ac_mpscfifo_rmv_msg_raw(&mp->mpscfifo);
    ac_debug_printf("AcMsg_get:-msg=%p\n", msg);
  }
  return msg;
}

/**
 * Ret a message to a pool
 *
 * @param msg a message to return the the pool, AC_NULL is ignored
 */
void AcMsg_ret(AcMsg* msg) {
  if ((msg != AC_NULL) && (msg->pool != AC_NULL)) {
    ac_mpscfifo_add_msg(&msg->pool->mpscfifo, msg);
    ac_debug_printf("AcMsg_ret:-msg=%p msg->pool=%p\n", msg, msg->pool);
  } else {
    ac_debug_printf("AcMsg_ret:+msg=%p msg->pool=%p\n", msg, msg != AC_NULL ? msg->pool : AC_NULL );
  }
}

/**
 * Create a msg pool
 *
 * @params msg_count is number of messages for this pool
 *
 * @return AC_NULL if no pool is created
 */
AcMsgPool* AcMsgPool_create(ac_u32 msg_count) {
  ac_bool error = AC_FALSE;
  AcMsgPool* pool = AC_NULL;
  AcMsg* arena = AC_NULL;

  if (msg_count == 0) {
    ac_debug_printf("AcMsgPool_create:-msg_count is 0 return pool=AC_NULL\n");
    goto done;
  }

  // Allocate the AcMsgPool
  pool = AcMsgPool_alloc();
  ac_debug_printf("AcMsgPool_create:+pool=%p\n", pool);
  if (pool == AC_NULL) {
    goto done;
  }

  // Allocate the AcMsg's and the stub
  arena = ac_malloc(sizeof(AcMsg) * (msg_count + 1));
  if (arena == AC_NULL) {
    goto done;
  }
  arena[0].pool = pool;
  ac_debug_printf("AcMsgPool_create:+ pool=%p arena=%p\n", pool, arena);

  // Initialize the mpscfifo with the first entry as the stub
  ac_mpscfifo_init_with_stub(&pool->mpscfifo, &arena[0]);

  // Add the rest to the pool
  for (ac_u32 i = 1; i <= msg_count; i++) {
    ac_mpscfifo_add_msg(&pool->mpscfifo, &arena[i]);
    arena[i].pool = pool;
    ac_debug_printf("AcMsgPool_create: adding &arena[%d]=%p pool=%p\n",
        i, &arena[i], arena[i].pool);
  }

done:
  ac_debug_printf("AcMsgPool_create:- pool=%p msg_count=%d\n",
      pool, msg_count);

  if (error) {
    if (arena != AC_NULL) {
      ac_free(arena);
      arena = AC_NULL;
    }
    if (pool != AC_NULL) {
      ac_free(pool);
      pool = AC_NULL;
    }
  }

  return pool;
}
