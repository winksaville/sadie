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
 * Debug code for AcMpscFifoIn
 */

#define NDEBUG

#include <ac_mpsc_fifo_in.h>
#include <ac_mpsc_fifo_in_dbg.h>
#include <ac_mpsc_fifo_in_internal.h>

#include <ac_mem.h>
#include <ac_mem_dbg.h>
#include <ac_printf.h>

/**
 * @see AcMpscFifoIn_dbg.h
 */
void AcMpscFifoIn_print(const char* leader, AcMpscFifoIn* fifo) {
  if (fifo != AC_NULL) {
    if (leader != AC_NULL) {
      ac_printf("%s %p\n", leader, fifo);
    }
#ifndef NDEBUG
    AcMem_print("fifo->head: ", fifo->head);
    AcMem_print("fifo->tail: ", fifo->tail);
#endif
    AcMem* tail = fifo->tail;
    AcMem* next = tail->hdr.next;
    if (next == AC_NULL) {
      if (tail == fifo->head) {
        AcMem_print("empty h/t: ", fifo->head);
      } else {
        AcMem_print("preempted tail==NULL head=: ", fifo->head);
      }
    } else if (next == fifo->head) {
      AcMem_print("one h/t:   ", fifo->head);
    } else {
      ac_bool first_time = AC_TRUE;
      while (next != AC_NULL) {
        if (first_time) {
          first_time = AC_FALSE;
          ac_printf("n tail:    ");
        } else {
          ac_printf("           ");
        }
        AcMem_print(AC_NULL, next);
        next = next->hdr.next;
      }
    }
  } else {
    ac_printf("fifo == AC_NULL");
  }
}