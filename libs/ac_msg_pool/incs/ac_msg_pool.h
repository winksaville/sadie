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
 * A ac_msg_pool contains msgs which can be used for sending
 * information between components.
 */

#ifndef SADIE_LIBS_AC_MSG_POOL_INCS_AC_MSG_POOL_H
#define SADIE_LIBS_AC_MSG_POOL_INCS_AC_MSG_POOL_H

#include <ac_inttypes.h>
#include <ac_msg.h>

typedef struct msg_pool* AcMsgPool;

/**
 * Create a msg pool
 *
 * @return AC_NULL if no pool
 */
AcMsgPool AcMsgPool_create(ac_u32 count);

/**
 * Get a message from a pool
 *
 * @param pool is a previously created pool
 *
 * @return a message or AC_NULL if none available
 */
AcMsg* AcMsg_get(AcMsgPool pool);

/**
 * Ret a message to a pool
 *
 * @param pool is a previously created pool
 * @param msg a message to return the the pool, AC_NULL is ignored
 */
void AcMsg_ret(AcMsg* msg);

#endif
