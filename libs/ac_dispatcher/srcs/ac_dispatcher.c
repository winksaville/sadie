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
#include <ac_mpsc_fifo.h>
#include <ac_msg.h>
#include <ac_memmgr.h>

/**
 * A Dispatchable Component
 */
typedef struct AcDispatchableComp {
    AcComp* comp;
    AcMpscFifo q;
} AcDispatchableComp;

/**
 * A dispatcher
 */
typedef struct AcDispatcher {
  ac_u32 max_count;
  AcDispatchableComp* dcs[];
} AcDispatcher;

/** d->AcDispatchableComp[i] is empty and can be reallocated */
#define DC_EMPTY       ((AcDispatchableComp*)(0))

/** d->AcDispatchableComp[i] messages are being processed by AcDispatcher */
#define DC_PROCESSING  ((AcDispatchableComp*)(1))

/**
 * Get a AcDispatchableComp aka dc
 */
AcDispatchableComp* get_dc() {
  // TODO: Pre allocate??
  AcDispatchableComp* ret_val = ac_malloc(sizeof(AcDispatchableComp));
  if (ret_val != AC_NULL) {
    AcMpscFifo_init(&ret_val->q, sizeof(AcMsg));
  }
  return ret_val;
}

/**
 * Return a AcDispatchableComp aka dc
 */
void ret_dc(AcDispatchableComp* dc) {
  // TODO: Pre allocate??
  if (dc != AC_NULL) {
    AcMpscFifo_deinit(&dc->q);
    ac_free(dc);
  }
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
                          + (max_count * sizeof(AcDispatchableComp*)));
  if (d != AC_NULL) {
      d->max_count = max_count;
  } else {
    ret_dispatcher(d);
  }

  ac_debug_printf("alloc_dispatcher:- d=%p\n", d);
  return d;
}


/*
 * Process all of the messages on the AcDispatchableComp
 * return AC_TRUE if one or more were processed.
 */
static ac_bool process_msgs(AcDispatchableComp* dc) {
  ac_debug_printf("process_msgs:+ dc=%p\n", dc);

  ac_bool processed_a_msg = AC_FALSE;
  AcMsg* pmsg = AcMpscFifo_rmv_msg(&dc->q);
  while (pmsg != AC_NULL) {
    dc->comp->process_msg(dc->comp, pmsg);

    // Get next message
    pmsg = AcMpscFifo_rmv_msg(&dc->q);
    processed_a_msg = AC_TRUE;
  }

  ac_debug_printf("process_msgs:- dc=%p processed_a_msg=%d\n",
      dc, processed_a_msg);
  return processed_a_msg;
}

/*
 * Remove the dc aka AcDispacableComponent at d->dcs[ac_idx]
 */
static void rmv_dc(AcDispatcher* d, int dc_idx) {
  ac_debug_printf("rmv_dc:+ d=%p dc_idx=%d\n", d, dc_idx);

  // Get dc atomically and set to DC_EMPTY so no more messages
  // will be added.
  AcDispatchableComp** pdc = &d->dcs[dc_idx];
  AcDispatchableComp* dc = __atomic_exchange_n(pdc, DC_EMPTY, __ATOMIC_ACQUIRE);

  // If dc is already empty we're done. If it's DC_PROCESSING then were
  // racing with ac_dispatch and lost, ac_dispatch will process the
  // messages and all will be well, because we've marked it empty
  // and ac_dispatch will not change it back.
  if ((dc != DC_EMPTY) && (dc != DC_PROCESSING)) {
    // Process the messages as its not empty and ac_dispatch
    // isn't alreday process.
    process_msgs(dc);

    ret_dc(dc);

    ac_debug_printf("rmv_dc:- REMOVED d=%p dc_idx=%d\n", d, dc_idx);
    return;
  }

  ac_debug_printf("rmv_ac:- ALREADY removed d=%p dc_idx=%d\n",
      d, dc_idx);
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

  for (int i = 0; i < d->max_count; i++) {
    // Mark this AcDispatchableComp that we're processing
    AcDispatchableComp** pq = &d->dcs[i];
    AcDispatchableComp* q = __atomic_exchange_n(pq, DC_PROCESSING,__ATOMIC_ACQUIRE);

    // If q == DC_EMPTY then this entry is already removed or
    // will be so we're just store DC_EMPTY
    //
    // If q == DC_PROCESSING then someone else is processing
    // this and we should do nothing.
    //
    // If q is nither than we're going to process the AcDispatchableComp.
    if (q == DC_EMPTY) {
      ac_debug_printf("ac_dispatch: skip empty entry d=%p i=%d\n",
          d, i);
       __atomic_store_n(pq, DC_EMPTY, __ATOMIC_RELEASE);
    } else if (q == DC_PROCESSING) {
      ac_debug_printf("ac_dispatch: skip busy entry d=%p i=%d\n",
          d, i);
    } else {
      ac_debug_printf("ac_dispatch: process msgs d=%p i=%d\n",
          d, i);
      processed_msgs = process_msgs(q);
      /*const*/ AcDispatchableComp* dc_processing = DC_PROCESSING;

      // Now restore the previous q if it is still DC_PROCESSING.
      ac_bool restored = __atomic_compare_exchange_n(
                          pq, &dc_processing, q,
                          AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
      if (!restored) {
        // It wasn't restored so the only possibility is that while
        // we were processing the messages rmv_dc was invoked and
        // q is now DC_EMPTY so we need to finish the removal.
        ac_debug_printf("ac_dispatch: ret_acq as we won race with rmv_dc"
            " d=%p i=%d\n", d, i);
        ret_dc(q);
      }
    }
  }

  ac_debug_printf("ac_dispatch:- d=%p processed_msgs=%d\n",
      d, processed_msgs);
  return processed_msgs;
}

/**
 * Get a dispatcher ready to be used and that can support
 * upto max_count AcDispatchableComp's.
 */
AcDispatcher* AcDispatcher_get(ac_u32 max_count) {
  ac_debug_printf("AcDispatcher_get:+ max_count=%d\n", max_count);

  AcDispatcher* d = get_dispatcher(max_count);
  if (d != AC_NULL) {
    for (int i = 0; i < d->max_count; i++) {
      d->dcs[i] = DC_EMPTY;
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
      rmv_dc(d, i);
    }
    ret_dispatcher(d);
  }

  ac_debug_printf("AcDispatcher_ret:- d=%p\n", d);
}

/**
 * Add the AcComp to this dispatcher
 *
 * @return: AcDispatableComp* or AC_NULL if an error,
 * this will occur if there are to many AcComp's registered.
 */
AcDispatchableComp* AcDispatcher_add_comp(AcDispatcher* d, AcComp* comp) {
  ac_debug_printf("AcDispatcher_add_comp:+ d=%p d->max_count=%d comp=%p\n",
      d, d->max_count, comp);

  if (d == AC_NULL) {
    ac_debug_printf("AcDispatcher_add_comp:- ERR no d"
        " d=%p comp=%p\n", d, comp);
    return AC_NULL;
  }

  // Get the AcDispatchableComp and initialize
  AcDispatchableComp* dc = get_dc();
  if (dc == AC_NULL) {
    ac_debug_printf("AcDispatcher_add_comp:- ERR no AcDispatchableComp's"
        " d=%p comp=%p\n", d, comp);
    return AC_NULL;
  }
  dc->comp = comp;

  // Find a slot in the array to save the dc
  for (int i = 0; i < d->max_count; i++) {
    AcDispatchableComp** pdc = &d->dcs[i];
    AcDispatchableComp* dc_empty = DC_EMPTY;
    ac_debug_printf("AcDispatcher_add_comp: i=%d *pdc=%p dc_empty=%p\n",
          i, *pdc, dc_empty);
    if (__atomic_compare_exchange_n(
           pdc, &dc_empty, dc,
           AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
      ac_debug_printf("AcDispatcher_add_comp:- OK d=%p comp=%p dc=%p\n",
          d, comp, dc);
      return dc;
    }
  }

  // Couldn't find a slot so return the AcDispatchableComp and return AC_NULL
  ac_debug_printf("AcDispatcher_add_comp:- ERR d full\n");
  ret_dc(dc);
  return AC_NULL;
}

/**
 * Remove all instances of this AcComp assoicated with this dispatcher
 *
 * return the AcComp or AC_NULL if this AcComp was not added.
 */
AcComp* AcDispatcher_rmv_comp(AcDispatcher* d, AcDispatchableComp* dc) {
  AcComp* ret_val = dc->comp;
  ac_debug_printf("AcDispatcher_rmv_comp:+ d=%p dc=%p\n", d, dc);

  ac_bool ok = AC_FALSE;

  if (d == AC_NULL) {
    ac_debug_printf("AcDispatcher_rmv_comp:- ERR no d\n");
    return AC_NULL;
  }

  if (dc == AC_NULL) {
    ac_debug_printf("AcDispatcher_rmv_comp:- ERR no dc\n");
    return AC_NULL;
  }

  for (int i = 0; i < d->max_count; i++) {
    AcDispatchableComp** pcur_dc = &d->dcs[i];
    AcDispatchableComp* cur_dc = __atomic_load_n(pcur_dc, __ATOMIC_ACQUIRE);
    if (cur_dc == DC_EMPTY) {
      continue;
    }
    // Not nice but will eventually succeed
    while (cur_dc == DC_PROCESSING) {
        cur_dc = __atomic_load_n(pcur_dc, __ATOMIC_ACQUIRE);
    }
    if (cur_dc == DC_EMPTY) {
      continue;
    }
    if (ret_val == cur_dc->comp) {
      ac_debug_printf("AcDispatcher_rmv_comp: OK removing d=%p comp=%p\n", d, ret_val);
      ok = AC_TRUE;
      rmv_dc(d, i);
    }
  }

  if (!ok) {
    ret_val = AC_NULL;
  }
  ac_debug_printf("AcDispatcher_rmv_comp:- d=%p comp=%p\n", d, ret_val);
  return ret_val;
}

/**
 * Send a message to dispatchable component
 *
 * @param: dc1 is the dispatchable component previously added.
 * @param: msg is the message to send
 */
void AcDispatcher_send_msg(AcDispatchableComp* dc, AcMsg* msg) {
  AcMpscFifo_add_msg(&dc->q, msg);
}
