/*
 * Copyright 2015 Wink Saville
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

#include <ac_dispatcher.h>

#include <ac_inttypes.h>
#include <ac_debug_printf.h>

#include <ac_memmgr.h>

typedef struct _acq {
    ac* pac;
    ac_mpscfifo* pq;
} acq;

typedef struct _ac_dispatcher {
  ac_u32 max_count;
  acq* acqs[];
} ac_dispatcher;

/** pd->acq[i] is empty and can be reallocated */
#define ACQ_EMPTY       ((acq*)(0))

/** pd->acq[i] messages are being processed by ac_dispatcher */
#define ACQ_PROCESSING  ((acq*)(!ACQ_EMPTY))

void ret_acq(acq* pacq) {
  // Only posix at the momment
  ac_free(pacq);
}

acq* get_acq() {
  // Only posix at the momment
  return ac_malloc(sizeof(acq));
}

/**
 * Return the dispatcher its no longer going to be used,
 * assume deinitialization has already be done.
 */
static void ret_dispatcher(ac_dispatcher* pd) {
  ac_debug_printf("free_dispatcher:+ pd=%p\n", pd);

  if (pd != AC_NULL) {
    ac_free(pd);
  }

  ac_debug_printf("free_dispatcher:- pd=%p\n", pd);
}

/**
 * Get a dispatcher to use.
 */
static ac_dispatcher* get_dispatcher(ac_u32 max_count) {
  ac_debug_printf("alloc_dispatcher:+ max_count=%d\n", max_count);

   ac_dispatcher* pd = ac_malloc(sizeof(ac_dispatcher)
                          + (max_count * sizeof(acq*)));
  if (pd != AC_NULL) {
      pd->max_count = max_count;
  } else {
    ret_dispatcher(pd);
  }

  ac_debug_printf("alloc_dispatcher:- pd=%p\n", pd);
  return pd;
}


/*
 * Process all of the messages on the queue
 * return AC_TRUE if one or more were processed.
 */
static ac_bool process_msgs(acq* pacq) {
  ac_debug_printf("process_msgs:+ pacq=%p\n", pacq);

  ac_bool processed_a_msg = AC_FALSE;
  ac_msg* pmsg = ac_rmv_msg(pacq->pq);
  while (pmsg != AC_NULL) {
    pacq->pac->process_msg(pacq->pac, pmsg);
    // TODO: return the message to a pool.

    // Get next message
    pmsg = ac_rmv_msg(pacq->pq);
    processed_a_msg = AC_TRUE;
  }

  ac_debug_printf("process_msgs:- pacq=%p\n", pacq);
  return processed_a_msg;
}

/*
 * Remove the ac at pd->acqs[ac_idx]
 */
static void rmv_acq(ac_dispatcher* pd, int acq_idx) {
  ac_debug_printf("rmv_ac:+ pd=%p cq_idx=%d\n", pd, acq_idx);

  // Get pacq atomically and set to ACQ_EMPTY so no more messages
  // will be added.
  acq** ppacq = &pd->acqs[acq_idx];
  acq* pacq = __atomic_exchange_n(ppacq, ACQ_EMPTY, __ATOMIC_ACQUIRE);

  // If pacq is already empty we're done. If its ACQ_PROCESSING then were
  // racing with ac_dispatch and lost, ac_dispatch will process the
  // messages and all will be well, because we've marked it empty
  // and ac_dispatch will not change it back.
  if ((pacq != ACQ_EMPTY) && (pacq != ACQ_PROCESSING)) {
    // Process the messages as its not empty and ac_dispatch
    // isn't alreday process.
    process_msgs(pacq);

    ret_acq(pacq);

    ac_debug_printf("rmv_ac:- REMOVED pd=%p acq_idx=%d\n", pd, acq_idx);
    return;
  }

  ac_debug_printf("rmv_ac:- ALREADY removed pd=%p ac_idx=%d\n",
      pd, acq_idx);
}

/**
 * Dispatch messages to asynchronous components
 */
void ac_dispatch(ac_dispatcher* pd) {
  ac_debug_printf("ac_dispatch:+ pd=%p\n", pd);

  if (pd == AC_NULL) {
    ac_debug_printf("ac_dispatch:- ERR no pd pd=%p\n", pd);
    return;
  }

  for (int i = 0; i < pd->max_count; i++) {
    // Mark this acq that we're processing
    acq** ppacq = &pd->acqs[i];
    acq* pacq = __atomic_exchange_n(ppacq, ACQ_PROCESSING,__ATOMIC_ACQUIRE);

    // If we're racing with ac_dispatcher_deinit and we lost then
    // pacq == ACQ_EMPTY and we won't do anything. If we won then
    // we've set it to ACQ_PROCESSING and we'll process the messages.
    if ((pacq != ACQ_EMPTY) && (pacq != ACQ_PROCESSING)) {
      // The acq is not empty and someone else isn't processing
      process_msgs(pacq);
    }

    // Restore pacq if ac_dispatcher_deinit didn't mark it ACQ_EMPTY
    static void* acq_processing = ACQ_PROCESSING;
    ac_bool restored = __atomic_compare_exchange_n(
                        &pacq, &acq_processing, pacq,
                        AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (!restored) {
      ac_debug_printf("ac_dispatch: ret_acq we won race pd=%p acq_idx=%d\n",
          pd, i);
      ret_acq(pacq);
    }
  }

  ac_debug_printf("ac_dispatch:- pd=%p\n", pd);
}

/**
 * Get a dispatcher ready to be used and that can support
 * uptor max_count acq's.
 */
ac_dispatcher* ac_dispatcher_get(ac_u32 max_count) {
  ac_debug_printf("ac_dispatcher_get:+ max_count=%d\n", max_count);

  ac_dispatcher* pd = get_dispatcher(max_count);
  if (pd != AC_NULL) {
    for (int i = 0; i < pd->max_count; i++) {
      pd->acqs[i] = ACQ_EMPTY;
    }
  }

  ac_debug_printf("ac_dispatcher_get:- max_count=%d pd=%p\n",
      max_count, pd);
  return pd;
}

/**
 * Return dispatcher, the dispatcher will no longer be valid for use.
 */
void ac_dispatcher_ret(ac_dispatcher* pd) {
  ac_debug_printf("ac_dispatcher_ret:+ pd=%p\n", pd);

  if (pd != AC_NULL) {
    for(int i = 0; i < pd->max_count; i++) {
      rmv_acq(pd, i);
    }
    ret_dispatcher(pd);
  }

  ac_debug_printf("ac_dispatcher_ret:- pd=%p\n", pd);
}

/**
 * Add the ac and its queue to this dispatcher
 *
 * return the pac or AC_NULL if this ac was not added
 * this will occur if there are to many ac's registered.
 */
ac* ac_dispatcher_add_acq(ac_dispatcher* pd, ac* pac, ac_mpscfifo* pq) {
  ac_debug_printf("ac_dispatcher_add_acq:+ pd=%p pac=%p pq=%p\n",
      pd, pac, pq);

  if (pd == AC_NULL) {
    ac_debug_printf("ac_dispatcher_add_acq:- ERR no pd"
        " pd=%p pac=%p pq=%p\n", pd, pac, pq);
    return AC_NULL;
  }

  acq* pacq = get_acq(); 
  if (pacq == AC_NULL) {
    ac_debug_printf("ac_dispatcher_add_acq:- ERR no acq's"
        " pd=%p pac=%p pq=%p\n", pd, pac, pq);
    return AC_NULL;
  }

  // Find a slot in the array to st
  for(int i = 0; i < pd->max_count; i++) {
    static acq* pacq_empty = ACQ_EMPTY;
    acq** ppacq = &pd->acqs[i];
    if (__atomic_compare_exchange_n(
           ppacq, &pacq_empty, pacq,
           AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
      ac_debug_printf("ac_dispatcher_add_acq:- OK"
          " pd=%p pac=%p pq=%p\n", pd, pac, pq);
      return pac;
    }
  }
  ac_debug_printf("ac_dispatcher_add_acq:- ERR pd full\n");
  ret_acq(pacq);
  return AC_NULL;
}

/**
 * Remove all instances of this ac assoicated with this dispatcher
 *
 * return the ac or AC_NULL if this ac was not added.
 */
ac* ac_dispatcher_rmv_acq(ac_dispatcher* pd, ac* pac) {
  ac_debug_printf("ac_dispatcher_rmv_acq:+ pd=%p pac=%p\n", pd, pac);

  ac_bool ok = AC_FALSE;

  if (pd == AC_NULL) {
    ac_debug_printf("ac_dispatcher_rmv_acq:- ERR no pd"
        " pd=%p pac=%p\n", pd, pac);
    return AC_NULL;
  }

  if (pac == AC_NULL) {
    ac_debug_printf("ac_dispatcher_rmv_acq:- ERR no ac"
        " pd=%p pac=%p\n", pd, pac);
    return AC_NULL;
  }

  for(int i = 0; i < pd->max_count; i++) {
    acq** ppacq = &pd->acqs[i];
    acq* pacq = __atomic_load_n(ppacq, __ATOMIC_ACQUIRE);
    if (pacq == ACQ_EMPTY) {
      continue;
    }
    // Not nice but will eventually succeed
    while (pacq == ACQ_PROCESSING) {
        pacq = __atomic_load_n(ppacq, __ATOMIC_ACQUIRE);
    }
    if (pacq == ACQ_EMPTY) {
      continue;
    }
    if (pac == pacq->pac) {
      ok = AC_TRUE;
      rmv_acq(pd, i);
    }
  }

  if (!ok) {
    pac = AC_NULL;
  }
  ac_debug_printf("ac_dispatcher_rmv_acq:- pd=%p pac=%p\n", pd, pac);
  return pac;
}
