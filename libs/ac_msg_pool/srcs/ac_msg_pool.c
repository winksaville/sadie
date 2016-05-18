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

#include <ac_msg.h>

struct msg_pool {
};

/**
 * Create a msg pool
 *
 * @return AC_NULL if no pool
 */
ac_msg_pool ac_msg_pool_create(ac_u32 count) {
  ac_msg_pool pool;

  if (count == 0) {
    pool = AC_NULL;
  } else {
    // TODO: implement
    pool = AC_NULL;
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
ac_msg* ac_msg_get(ac_msg_pool mp) {
  ac_msg* msg;

  if (mp == AC_NULL) {
    msg = AC_NULL;
  } else {
    // TODO: implement
    msg = AC_NULL;
  }
  return AC_NULL;
}

/**
 * Ret a message to a pool
 *
 * @param msg a message to return the the pool, AC_NULL is ignored
 */
void ac_msg_ret(ac_msg_pool mp, ac_msg* msg) {
  if ((mp != AC_NULL) && (msg != AC_NULL)) {
    // TODO: implement
  }
}
