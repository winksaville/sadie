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

//#define NDEBUG

#include <ac_comp_mgr.h>

#include <ac_assert.h>
#include <ac_dispatcher.h>
#include <ac_memmgr.h>
#include <ac_mpscfifo.h>
#include <ac_msg.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_receptor.h>
#include <ac_thread.h>

/**
 * A opaque component info for an AcComp
 */
typedef struct AcCompInfo {
  AcComp* comp;
  AcDispatchableComp* dc;
  ac_u32 idx;
} AcCompInfo;

/**
 * Array of AcCompInfo objects being managed across all of the threads
 */
static AcCompInfo* comp_infos;

/**
 * Parameters for each thread to manage its components
 */
typedef struct DispatchThreadParams {
  ac_u32 max_comps;
  AcCompInfo** comps;
  ac_receptor_t done;
  ac_receptor_t ready;
  ac_receptor_t waiting;
  ac_bool stop_processing_msgs;
} DispatchThreadParams;

/**
 * Array of DispathThreadParams, one for each thread
 */
static DispatchThreadParams* dtps;

/**
 * A thread which dispatches message to its components.
 */
static void* dispatch_thread(void *param) {
  DispatchThreadParams* params = (DispatchThreadParams*)(param);
  AcDispatcher* d;

  ac_debug_printf("dispatch_thread:+ starting params=%p\n", params);

  // Get a dispatcher and add a queue and message processor
  d = AcDispatcher_get(params->max_comps);
  if (d == AC_NULL) {
    goto done;
  }

  // Create the waiting receptor and init our not stopped flag
  params->waiting = ac_receptor_create();
  __atomic_store_n(&params->stop_processing_msgs, AC_FALSE, __ATOMIC_RELEASE);

  // Signal dispatch_thread is ready
  ac_receptor_signal(params->ready);

  // Continuously dispatch messages until we're told to stop
  while (__atomic_load_n(&params->stop_processing_msgs, __ATOMIC_ACQUIRE) == AC_FALSE) {
    if (!AcDispatcher_dispatch(d)) {
      ac_debug_printf("dispatch_thread: waiting\n");
      ac_receptor_wait(params->waiting);
      ac_debug_printf("dispatch_thread: continuing\n");
    }
  }

  // Cleanup
  for (ac_u32 j = 0; j < params->max_comps; j++) {
    AcCompInfo* ci = params->comps[j];
    // TODO: cleanup waiting receptors
    if (ci != AC_NULL) {
      if (ci->dc != AC_NULL) {
        AcDispatcher_rmv_comp(d, ci->dc);
      }
      ac_free(ci);
      params->comps[j] = AC_NULL;
    }
  }

  ac_debug_printf("disptach_thread:- done params=%p\n", params);

  ac_receptor_signal_yield_if_waiting(params->done);

done:
  if (d != AC_NULL) {
    AcDispatcher_ret(d);
  }
  return AC_NULL;
}

/**
 * Initialize the component manager, may only be called once.
 *
 * @param: max_component_threads is the maximum number of threads to manage
 * @param: max_components_per_thread is the maximum number of components per thread
 * @param: stack_size is number of bytes for a threads stack, 0 will provide the default
 */
void AcCompMgr_init(ac_u32 max_component_threads, ac_u32 max_components_per_thread,
    ac_u32 stack_size) {
  ac_debug_printf("AcCompMgr_init:+ max_component_threads=%d max_components_per_thread=%d stack_size=%d\n",
      max_component_threads, max_components_per_thread, stack_size);
      
  if (max_component_threads == 0) {
    ac_printf("Counld not create the AcCompMgr max_component_threads is 0\n");
    return;
  }

  if (max_components_per_thread == 0) {
    ac_printf("Counld not create the AcCompMgr max_components_per_thread is 0\n");
    return;
  }

  dtps = ac_malloc(max_component_threads * sizeof(DispatchThreadParams));
  ac_assert(dtps != AC_NULL);
  comp_infos = ac_malloc(max_component_threads * max_components_per_thread * sizeof(AcCompInfo));
  ac_assert(comp_infos != AC_NULL);


  ac_debug_printf("AcCompMgr_init: 1\n");
  for (ac_u32 i = 0; i < max_component_threads; i++) {
    ac_debug_printf("AcCompMgr_init: i=%d\n", i);
    DispatchThreadParams* dtp = &dtps[i];
    dtp->max_comps = max_components_per_thread;
    dtp->comps = ac_malloc(max_components_per_thread * sizeof(AcCompInfo*));
    if (dtp->comps == AC_NULL) {
      ac_debug_printf("AcCompMgr_init: i=%d\n dtp->comps == AC_NULL", i);
      // TODO: Better cleanup
      return;
    }
    for (ac_u32 j = 0; j < max_components_per_thread; j++) {
      ac_u32 ci_idx = (i * max_component_threads) + j;
      ac_debug_printf("AcCompMgr_init: i=%d j=%d ci_idx=%d\n", i, j, ci_idx);
      AcCompInfo* ci = &comp_infos[ci_idx];
      dtp->comps[j] = ci;
      ci->comp = AC_NULL;
      ci->dc = AC_NULL;
    }
    dtp->done = ac_receptor_create();
    dtp->ready = ac_receptor_create();

    ac_thread_rslt_t result = ac_thread_create(stack_size, dispatch_thread, dtp);
    if (result.status != 0) {
      ac_printf("Counld not create the dispatch_thread %d result.status=%d\n",
          i, result.status);
      // TODO: Better cleanup
      return;
    }

    // Wait until the thread is ready
    ac_receptor_wait(dtp->ready);
  }
}

/**
 * Deinitialzie the component manager.
 */
void AcCompMgr_deinit(void) {
}

/**
 * Add a component to be managed
 *
 * @param: comp is an initialized component type
 *
 * @return: AcCompId or AC_NULL if an error
 */
AcCompInfo* AcCompMgr_add_comp(AcComp* comp) {
  return AC_NULL;
}

/**
 * Remove a component being managed
 *
 * @param: comp is an initialized component type
 */
void AcCompMgr_rmv_comp(AcCompInfo* info) {
}
