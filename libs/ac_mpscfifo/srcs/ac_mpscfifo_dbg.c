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
 * Debug code for mpscfifo
 */

#define NDEBUG

#include <ac_printf.h>

#include "ac_mpscfifo_dbg.h"

/**
 * @see ac_mpscfifo_dbg.h
 */
void ac_msg_print(const char* leader, AcMsg* pmsg) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  if (pmsg != AC_NULL) {
    ac_printf("pmsg=%p pnext=%p pool=%p arg1=0x%lx arg2=0x%lx\n",
        (void *)pmsg, (void *)(pmsg->pnext), pmsg->pool, pmsg->arg1, pmsg->arg2);
  } else {
    ac_printf("pmsg == AC_NULL\n");
  }
}

/**
 * @see ac_mpscfifo_dbg.h
 */
void ac_mpscfifo_print(ac_mpscfifo* pq) {
  if (pq != AC_NULL) {
#ifndef NDEBUG
    ac_msg_print("pq->phead: ", pq->phead);
    ac_msg_print("pq->ptail: ", AC_NULL, pq->ptail);
#endif
    AcMsg* ptail = pq->ptail->pnext;
    if (ptail == AC_NULL) {
      ac_msg_print("empty h/t: ", pq->phead);
    } else if (ptail == pq->phead) {
      ac_msg_print("one h/t:   ", pq->phead);
    } else {
      ac_bool first_time = AC_TRUE;
      while (ptail != AC_NULL) {
        if (first_time) {
          first_time = AC_FALSE;
          ac_printf("n ptail:   ");
        } else {
          ac_printf("           ");
        }
        ac_msg_print(AC_NULL, ptail);
        ptail = ptail->pnext;
      }
    }
  } else {
    ac_printf("pq == AC_NULL");
  }
}
