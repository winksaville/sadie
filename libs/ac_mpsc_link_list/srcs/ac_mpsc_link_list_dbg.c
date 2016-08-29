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

#include <ac_mpsc_link_list.h>
#include <ac_mpsc_link_list_dbg.h>

#include <ac_msg.h>
#include <ac_printf.h>

void AcNextPtr_print(const char* leader, AcNextPtr* next) {
  if (leader == AC_NULL) {
    leader = "";
  }
  ac_printf("%s next=%p", leader, next);
  if (next != AC_NULL) {
    ac_printf(" .next=%p .msg=%p", next->next, next->msg);
  }
  ac_printf("\n");
}

/**
 * @see ac_mspc_link_list_dbg.h
 */
void AcMpscLinkList_print(const char* leader, AcMpscLinkList* list) {
  if (list != AC_NULL) {
    if (leader != AC_NULL) {
      ac_printf("%s %p\n", leader, list);
    }
#ifndef NDEBUG
    AcNextPtr_print("list->head: ", list->head);
    AcNextPtr_print("list->tail: ", list->tail);
#endif
    AcNextPtr* tail = list->tail;
    AcNextPtr* next_ptr = list->tail->next;
    if (next_ptr == AC_NULL) {
      if (tail == list->head) {
        AcNextPtr_print("empty h/t: ", list->head);
      } else {
        AcNextPtr_print("preempted: ", list->head);
      }
    } else if (next_ptr == list->head) {
      AcNextPtr_print("one h/t:   ", list->head);
    } else {
      AcBool first_time = AC_TRUE;
      while (next_ptr != AC_NULL) {
        if (first_time) {
          first_time = AC_FALSE;
          ac_printf("n tail:    ");
        } else {
          ac_printf("           ");
        }
        AcNextPtr_print(AC_NULL, next_ptr);
        next_ptr = next_ptr->next;
      }
    }
  } else {
    ac_printf("list == AC_NULL");
  }
}
