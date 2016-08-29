/*
 * This software is released into the public domain.
 *
 * A AcMpscLinkList is a wait free/thread safe multi-producer
 * single consumer first in first out queue using a link list.
 * This algorithm is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 */

#define NDEBUG

#include <ac_mpsc_link_list.h>
#include <ac_mpsc_link_list_internal.h>

#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_thread.h>

#ifdef NDEBUG
#define COUNTERS 0
#else
#define COUNTERS 1
#endif

/**
 * @see ac_mpsc_link_list.h
 */
AcStatus AcMpscLinkList_init(AcMpscLinkList* list) {
  ac_debug_printf("AcMpscLinkList_init:+list=%p\n", list);

  list->stub.next = AC_NULL;
  list->stub.msg = AC_NULL;
  list->head = &list->stub;
  list->tail = &list->stub;
  list->count = 0;
  list->msgs_processed = 0;

  ac_debug_printf("AcMpscLinkList_init:-list=%p status=AC_STATUS_OK\n", list);
  return AC_STATUS_OK;
}

/**
 * @see ac_mpsc_link_list.h
 */
AcU64 AcMpscLinkList_deinit(AcMpscLinkList* list) {
  ac_debug_printf("AcMpscLinkList_deinit:+list=%p\n", list);

  AcU64 msgs_processed = list->msgs_processed;
#ifndef NDEBUG
  AcU32 count = list->count;
#endif

  AcNextPtr* next_ptr = list->head;
  next_ptr->next = AC_NULL;
  list->head = AC_NULL;
  list->tail = AC_NULL;
  list->count = 0;
  list->msgs_processed = 0;

  ac_debug_printf("AcMpscLinkList_deinit:-list=%p count=%u msgs_processed=%lu\n", list, count, msgs_processed);
  return msgs_processed;
}

/**
 * @see ac_mpsc_link_list.h
 */
void AcMpscLinkList_add(AcMpscLinkList* list, AcMsg* msg) {
  ac_debug_printf("AcMpscLinkList_add:+list=%p msg=%p\n", list, msg);

  AcNextPtr* next_ptr = msg->next_ptr;
  next_ptr->next = AC_NULL;
  next_ptr->msg = msg;
  AcNextPtr* prev = __atomic_exchange_n(&list->head, next_ptr, __ATOMIC_ACQ_REL);
  // rmv will stall spinning if preempted at this critical spot
  __atomic_store_n(&prev->next, next_ptr, __ATOMIC_RELEASE);

#if COUNTERS
  list->count += 1;
#endif

  ac_debug_printf("AcMpscLinkList_add:-list=%p msg=%p\n", list, msg);
}

/**
 * @see ac_mpsc_link_list.h
 */
AcMsg* AcMpscLinkList_rmv(AcMpscLinkList* list) {
  ac_debug_printf("AcMpscLinkList_rmv:+list=%p\n", list);

  AcMsg* msg;
  AcNextPtr* tail = list->tail;
  AcNextPtr* next = tail->next;
  if ((next == AC_NULL) && (tail == __atomic_load_n(&list->head, __ATOMIC_ACQUIRE))) {
    ac_debug_printf("AcMpscLinkList_rmv:-list=%p EMPTY\n", list);
    return AC_NULL;
  } else {
    if (next == AC_NULL) {
      while ((next = __atomic_load_n(&tail->next, __ATOMIC_ACQUIRE)) == AC_NULL) {
        ac_thread_yield();
      }
    }
    msg = next->msg;
    msg->next_ptr = tail;
    list->tail = next;
    if (msg == AC_NULL) {
      ac_fail("AcMpscLinkList_rmv: msg == AC_NULL");
    }
#if COUNTERS
    list->count -= 1;
    list->msgs_processed += 1;
#endif
    ac_debug_printf("AcMpscLinkList_rmv:-list=%p msg=%p\n", list, msg);
    return msg;
  }
}
