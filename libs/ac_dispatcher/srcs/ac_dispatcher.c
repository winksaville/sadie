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

// TODO: Subset of AcCompInfo in ac_comp_mgr, fix!!!
typedef struct queue {
    AcComp* comp;
    ac_mpscfifo* q;
} queue;

typedef struct AcDispatcher {
  ac_u32 max_count;
  queue* acqs[];
} AcDispatcher;

/** d->queue[i] is empty and can be reallocated */
#define ACQ_EMPTY       ((queue*)(0))

/** d->queue[i] messages are being processed by AcDispatcher */
#define ACQ_PROCESSING  ((queue*)(!ACQ_EMPTY))

void ret_acq(queue* q) {
  // Only posix at the momment
  ac_free(q);
}

queue* get_queue() {
  // Only posix at the momment
  return ac_malloc(sizeof(queue));
}

/**
 * Return the dispatcher its no longer going to be used,
 * assume deinitialization has already be done.
 */
static void ret_dispatcher(AcDispatcher* d) {
  ac_debug_printf("free_dispatcher:+ d=%p\n", d);

  if (d != AC_NULL) {
    ac_free(d);
  }

  ac_debug_printf("free_dispatcher:- d=%p\n", d);
}

/**
 * Get a dispatcher to use.
 */
static AcDispatcher* get_dispatcher(ac_u32 max_count) {
  ac_debug_printf("alloc_dispatcher:+ max_count=%d\n", max_count);

   AcDispatcher* d = ac_malloc(sizeof(AcDispatcher)
                          + (max_count * sizeof(queue*)));
  if (d != AC_NULL) {
      d->max_count = max_count;
  } else {
    ret_dispatcher(d);
  }

  ac_debug_printf("alloc_dispatcher:- d=%p\n", d);
  return d;
}


/*
 * Process all of the messages on the queue
 * return AC_TRUE if one or more were processed.
 */
static ac_bool process_msgs(queue* q) {
  ac_debug_printf("process_msgs:+ q=%p\n", q);

  ac_bool processed_a_msg = AC_FALSE;
  ac_msg* pmsg = ac_mpscfifo_rmv_msg(q->q);
  while (pmsg != AC_NULL) {
    q->comp->process_msg(q->comp, pmsg);
    // TODO: return the message to a pool.

    // Get next message
    pmsg = ac_mpscfifo_rmv_msg(q->q);
    processed_a_msg = AC_TRUE;
  }

  ac_debug_printf("process_msgs:- q=%p processed_a_msg=%d\n",
      q, processed_a_msg);
  return processed_a_msg;
}

/*
 * Remove the AcComp at d->acqs[ac_idx]
 */
static void rmv_acq(AcDispatcher* d, int acq_idx) {
  ac_debug_printf("rmv_acq:+ d=%p cq_idx=%d\n", d, acq_idx);

  // Get q atomically and set to ACQ_EMPTY so no more messages
  // will be added.
  queue** pq = &d->acqs[acq_idx];
  queue* q = __atomic_exchange_n(pq, ACQ_EMPTY, __ATOMIC_ACQUIRE);

  // If q is already empty we're done. If its ACQ_PROCESSING then were
  // racing with ac_dispatch and lost, ac_dispatch will process the
  // messages and all will be well, because we've marked it empty
  // and ac_dispatch will not change it back.
  if ((q != ACQ_EMPTY) && (q != ACQ_PROCESSING)) {
    // Process the messages as its not empty and ac_dispatch
    // isn't alreday process.
    process_msgs(q);

    ret_acq(q);

    ac_debug_printf("rmv_acq:- REMOVED d=%p acq_idx=%d\n", d, acq_idx);
    return;
  }

  ac_debug_printf("rmv_ac:- ALREADY removed d=%p ac_idx=%d\n",
      d, acq_idx);
}

/**
 * Dispatch messages to asynchronous components
 *
 * CAUTION: This supports multiple dispatcher working on the same
 * list although this is untested as of now and we may want to
 * disallow it!!!
 */
ac_bool AcDispatcher_dispatch(AcDispatcher* d) {
  ac_bool processed_msgs = AC_FALSE;
  ac_debug_printf("ac_dispatch:+ d=%p\n", d);

  if (d == AC_NULL) {
    ac_debug_printf("ac_dispatch:- ERR no d d=%p\n", d);
    return processed_msgs;
  }

  const queue* acq_processing = ACQ_PROCESSING;
  for (int i = 0; i < d->max_count; i++) {
    // Mark this queue that we're processing
    queue** pq = &d->acqs[i];
    queue* q = __atomic_exchange_n(pq, ACQ_PROCESSING,__ATOMIC_ACQUIRE);

    // If q == ACQ_EMPTY then this entry is already removed or
    // will be so we're just store ACQ_EMPTY
    //
    // If q == ACQ_PROCESSING then someone else is processing
    // this and we should do nothing.
    //
    // If q is nither than we're going to process the queue.
    if (q == ACQ_EMPTY) {
      ac_debug_printf("ac_dispatch: skip empty entry d=%p acq_idx=%d\n",
          d, i);
       __atomic_store_n(pq, ACQ_EMPTY, __ATOMIC_RELEASE);
    } else if (q == ACQ_PROCESSING) {
      ac_debug_printf("ac_dispatch: skip busy entry d=%p acq_idx=%d\n",
          d, i);
    } else {
      ac_debug_printf("ac_dispatch: process msgs d=%p acq_idx=%d\n",
          d, i);
      processed_msgs = process_msgs(q);

      // Now restore the previous q if it is still ACQ_PROCESSING.
      ac_bool restored = __atomic_compare_exchange_n(
                          pq, &acq_processing, q,
                          AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
      if (!restored) {
        // It wasn't restored so the only possibility is that while
        // we were processing the messages rmv_acq was invoked and
        // q is now ACQ_EMPTY so we need to finish the removal.
        ac_debug_printf("ac_dispatch: ret_acq as we won race with rmv_acq"
            " d=%p acq_idx=%d\n", d, i);
        ret_acq(q);
      }
    }
  }

  ac_debug_printf("ac_dispatch:- d=%p processed_msgs=%d\n",
      d, processed_msgs);
  return processed_msgs;
}

/**
 * Get a dispatcher ready to be used and that can support
 * uptor max_count queue's.
 */
AcDispatcher* AcDispatcher_get(ac_u32 max_count) {
  ac_debug_printf("AcDispatcher_get:+ max_count=%d\n", max_count);

  AcDispatcher* d = get_dispatcher(max_count);
  if (d != AC_NULL) {
    for (int i = 0; i < d->max_count; i++) {
      d->acqs[i] = ACQ_EMPTY;
    }
  }

  ac_debug_printf("AcDispatcher_get:- max_count=%d d=%p\n",
      max_count, d);
  return d;
}

/**
 * Return dispatcher, the dispatcher will no longer be valid for use.
 */
void AcDispatcher_ret(AcDispatcher* d) {
  ac_debug_printf("AcDispatcher_ret:+ d=%p\n", d);

  if (d != AC_NULL) {
    for (int i = 0; i < d->max_count; i++) {
      rmv_acq(d, i);
    }
    ret_dispatcher(d);
  }

  ac_debug_printf("AcDispatcher_ret:- d=%p\n", d);
}

/**
 * Add the AcComp and its queue to this dispatcher
 *
 * return the comp or AC_NULL if this AcComp was not added
 * this will occur if there are to many AcComp's registered.
 */
AcComp* AcDispatcher_add_comp(AcDispatcher* d, AcComp* comp, ac_mpscfifo* fifo) {
  ac_debug_printf("AcDispatcher_add_acq:+ d=%p comp=%p fifo=%p\n",
      d, comp, fifo);

  if (d == AC_NULL) {
    ac_debug_printf("AcDispatcher_add_acq:- ERR no d"
        " d=%p comp=%p fifo=%p\n", d, comp, fifo);
    return AC_NULL;
  }

  // Get the queue and initialize
  queue* q = get_queue(); 
  if (q == AC_NULL) {
    ac_debug_printf("AcDispatcher_add_acq:- ERR no queue's"
        " d=%p comp=%p fifo=%p\n", d, comp, fifo);
    return AC_NULL;
  }
  q->comp = comp;
  q->q = fifo;

  // Find a slot in the array to save the q
  const queue* acq_empty = ACQ_EMPTY;
  for (int i = 0; i < d->max_count; i++) {
    queue** fifo = &d->acqs[i];
    if (__atomic_compare_exchange_n(
           fifo, &acq_empty, q,
           AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
      ac_debug_printf("AcDispatcher_add_acq:- OK"
          " d=%p comp=%p fifo=%p\n", d, comp, fifo);
      return comp;
    }
  }

  // Couldn't find a slot so return the queue and return AC_NULL
  ac_debug_printf("AcDispatcher_add_acq:- ERR d full\n");
  ret_acq(q);
  return AC_NULL;
}

/**
 * Remove all instances of this AcComp assoicated with this dispatcher
 *
 * return the AcComp or AC_NULL if this AcComp was not added.
 */
AcComp* AcDispatcher_rmv_comp(AcDispatcher* d, AcComp* comp) {
  ac_debug_printf("AcDispatcher_rmv_ac:+ d=%p comp=%p\n", d, comp);

  ac_bool ok = AC_FALSE;

  if (d == AC_NULL) {
    ac_debug_printf("AcDispatcher_rmv_ac:- ERR no d"
        " d=%p comp=%p\n", d, comp);
    return AC_NULL;
  }

  if (comp == AC_NULL) {
    ac_debug_printf("AcDispatcher_rmv_ac:- ERR no AcComp"
        " d=%p comp=%p\n", d, comp);
    return AC_NULL;
  }

  for (int i = 0; i < d->max_count; i++) {
    queue** pq = &d->acqs[i];
    queue* q = __atomic_load_n(pq, __ATOMIC_ACQUIRE);
    if (q == ACQ_EMPTY) {
      continue;
    }
    // Not nice but will eventually succeed
    while (q == ACQ_PROCESSING) {
        q = __atomic_load_n(pq, __ATOMIC_ACQUIRE);
    }
    if (q == ACQ_EMPTY) {
      continue;
    }
    if (comp == q->comp) {
      ac_debug_printf("AcDispatcher_rmv_ac: OK removing comp=%p pq=%p\n",
          comp, q->pq);
      ok = AC_TRUE;
      rmv_acq(d, i);
    }
  }

  if (!ok) {
    comp = AC_NULL;
  }
  ac_debug_printf("AcDispatcher_rmv_ac:- d=%p comp=%p\n", d, comp);
  return comp;
}
