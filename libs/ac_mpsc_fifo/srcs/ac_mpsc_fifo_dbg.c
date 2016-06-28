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

#include <ac_mpsc_fifo.h>
#include <ac_mpsc_fifo_dbg.h>

#include <ac_mem.h>
#include <ac_mem_dbg.h>
#include <ac_printf.h>

/**
 * @see AcMpscFifo_dbg.h
 */
void AcMpscFifo_print(const char* leader, AcMpscFifo* fifo) {
  if (fifo != AC_NULL) {
    if (leader == AC_NULL) {
      ac_printf("fifo=%p\n", fifo);
    } else {
      ac_printf("%s %p\n", leader, fifo);
    }
#ifndef NDEBUG
    AcMem_print("fifo->head: ", fifo->head);
    AcMem_print("fifo->tail: ", fifo->tail);
#endif
    AcMem* tail = fifo->tail->hdr.next;
    if (tail == AC_NULL) {
      AcMem_print("empty h/t: ", fifo->head);
    } else if (tail == fifo->head) {
      AcMem_print("one h/t:   ", fifo->head);
    } else {
      ac_bool first_time = AC_TRUE;
      while (tail != AC_NULL) {
        if (first_time) {
          first_time = AC_FALSE;
          ac_printf("n tail:    ");
        } else {
          ac_printf("           ");
        }
        AcMem_print(AC_NULL, tail);
        tail = tail->hdr.next;
      }
    }
  } else {
    ac_printf("fifo == AC_NULL");
  }
}
