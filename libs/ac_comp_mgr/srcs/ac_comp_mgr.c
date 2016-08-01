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

#include <ac_comp_mgr.h>

#include <ac_assert.h>
#include <ac_dispatcher.h>
#include <ac_memmgr.h>
#include <ac_msg.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_receptor.h>
#include <ac_string.h>
#include <ac_thread.h>

#if AC_PLATFORM == pc_x86_64
extern void remove_zombies(void);
#endif

typedef struct DispatchThreadParams DispatchThreadParams;

/**
 * A opaque component info for an AcComp
 */
typedef struct AcCompInfo {
  AcCompMgr* mgr;
  AcComp* comp;
  AcDispatchableComp* dc;
  ac_u32 comp_idx;
  DispatchThreadParams* dtp;
} AcCompInfo;

/**
 * Parameters for each thread to manage its components
 */
typedef struct DispatchThreadParams {
  ac_bool thread_started;
  ac_thread_hdl_t thread_hdl;
  AcDispatcher* d;
  ac_u32 max_comps;
  AcCompInfo** cis;
  AcReceptor* done;
  AcReceptor* ready;
  AcReceptor* waiting;
  ac_bool stop_processing_msgs;
} DispatchThreadParams;

typedef struct AcCompMgr {
  AcCompInfo* comp_infos;     // Array of AcCompInfo objects being managed
                              // across all of the threads
  ac_u32 comp_infos_max_count;// Number of elements in comp_infos array
  DispatchThreadParams* dtps; // Array of DispathThreadParams, one for each thread
  ac_u32 max_dtps;            // Number of threads in the dtps array
  ac_u32 next_dtps;           // Next thread
} AcCompMgr;

/**
 * A thread which dispatches message to its components.
 */
static void* dispatch_thread(void *param) {
  DispatchThreadParams* params = (DispatchThreadParams*)(param);

  ac_debug_printf("dispatch_thread:+starting params=%p\n", params);

  // Get a dispatcher and add a queue and message processor
  params->d = AcDispatcher_get(params->max_comps);
  if (params->d == AC_NULL) {
    ac_debug_printf("dispatch_thread: no AcDispatcher params=%p\n", params);
    goto done;
  }

  // Create the waiting receptor and init our not stopped flag
  params->waiting = AcReceptor_get();
  ac_assert(params->waiting != AC_NULL);
  __atomic_store_n(&params->stop_processing_msgs, AC_FALSE, __ATOMIC_RELEASE);

  // Signal dispatch_thread is ready
  AcReceptor_signal(params->ready);

  // Continuously dispatch messages until we're told to stop
  while (__atomic_load_n(&params->stop_processing_msgs, __ATOMIC_ACQUIRE) == AC_FALSE) {
    if (!AcDispatcher_dispatch(params->d)) {
      ac_debug_printf("dispatch_thread: waiting\n");
      AcReceptor_wait(params->waiting);
      ac_debug_printf("dispatch_thread: continuing\n");
    }
  }

  // Cleanup
  for (ac_u32 j = 0; j < params->max_comps; j++) {
    AcCompInfo* ci = params->cis[j];
    if (ci != AC_NULL) {
      if (ci->dc != AC_NULL) {
        AcDispatcher_rmv_comp(params->d, ci->dc);
      }
      params->cis[j] = AC_NULL;
    }
  }
  AcReceptor_ret(params->waiting);

done:
  ac_debug_printf("disptach_thread:-done params=%p\n", params);

  if (params->d != AC_NULL) {
    AcDispatcher_ret(params->d);
    params->d = AC_NULL;
    ac_debug_printf("disptach_thread:-params->d = AC_NULL ************\n");
  }

  AcReceptor_signal_yield_if_waiting(params->done);

  __atomic_thread_fence(__ATOMIC_RELEASE);
  return AC_NULL;
}

/**
 * see ac_comp_mgr.h
 */
AcComp* AcCompMgr_find_comp(AcCompMgr* mgr, ac_u8* name) {
  AcCompInfo* ci = AC_NULL;
  AcComp* comp = AC_NULL;
  ac_bool found = AC_FALSE;

  // Search the list CompInfos for the name
  for (ac_u32 i = 0; i < mgr->comp_infos_max_count; i++) {
    ci = &mgr->comp_infos[i];
    comp = ci->comp;
    if (comp != AC_NULL) {
      if (ac_strncmp((const char*)name, (const char*)comp->name, ac_strlen((const char*)comp->name)) == 0) {
        ac_debug_printf("AcCompMgr_find_comp: dtp->d=%p, could not add comp=%p\n", dtp->d, comp);
        found = AC_TRUE;
        break;
      }
    }
  }

  if (!found) {
    comp = AC_NULL;
  }
  ac_debug_printf("AcCompMgr_find_comp:-comp=%p\n", comp);
  return comp;
}

/**
 * see ac_comp_mgr.h
 */
AcCompInfo* AcCompMgr_add_comp(AcCompMgr* mgr, AcComp* comp) {
  AcCompInfo* ci = AC_NULL;
  ac_bool found = AC_FALSE;

  for (ac_u32 thrd = 0; !found && (thrd < mgr->max_dtps); thrd++) {
    // Search the dtps
    ac_u32 idx = __atomic_fetch_add(&mgr->next_dtps, 1, __ATOMIC_RELEASE);
    idx %= mgr->max_dtps;

    DispatchThreadParams* dtp = &mgr->dtps[idx];
    ac_debug_printf("AcCompMgr_add_comp:+comp=%p idx=%d dtp=%p dtp->d=%p dtp->max_comps=%d\n",
        comp, idx, dtp, dtp->d, dtp->max_comps);

    // Search the list CompInfos for an unused slot,
    // i.e. ci->comp == AC_NULL
    for (ac_u32 i = 0; i < dtp->max_comps; i++) {
      ci = dtp->cis[i];
      AcComp** pcomp = &ci->comp;
      AcComp* null_comp = AC_NULL;

      ac_debug_printf("AcCompMgr_add_comp: i=%d *pcomp=%p comp=%p\n", i, *pcomp, comp);
      if (__atomic_compare_exchange_n(pcomp, &null_comp, comp, AC_TRUE,
            __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
        ac_debug_printf("AcCompMgr_add_comp: i=%d found empty *pcomp=%p comp=%p\n", i, *pcomp, comp);
        // Found an empty entry add the component to the dispatcher
        ci->dc = AcDispatcher_add_comp(dtp->d, comp);
        if (ci->dc == AC_NULL) {
          ac_debug_printf("AcCompMgr_add_comp: dtp->d=%p, could not add comp=%p\n", dtp->d, comp);
          ci = AC_NULL;
        } else {
          found = AC_TRUE;
          break;
        }
      } else {
        // Not empty
        ci = AC_NULL;
      }
    }
  }

  ac_debug_printf("AcCompMgr_add_comp:-comp=%p ci=%p\n", comp, ci);
  return ci;
}

/**
 * see ac_comp_mgr.h
 */
AcComp* AcCompMgr_rmv_comp(AcCompMgr* mgr, AcCompInfo* ci) {
  AC_UNUSED(mgr);

  AcComp* comp = ci->comp;
  ac_debug_printf("AcCompMgr_rmv_comp:+ci=%p ci->dtp-d=%p comp=%p\n", ci, ci->dtp->d, comp);
  AcComp** pcomp = &ci->comp;
  if (__atomic_compare_exchange_n(pcomp, pcomp, AC_NULL, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
    ac_debug_printf("AcCompMgr_rmv_comp: removing comp=%p\n", comp);
    AcDispatcher_rmv_comp(ci->dtp->d, ci->dc);
    ci->dc = AC_NULL;
  }
  ac_debug_printf("AcCompMgr_rmv_comp:-ci=%p ret comp=%p\n", ci, comp);
  return comp;
}


/**
 * see ac_comp_mgr.h
 */
void AcCompMgr_send_msg(AcCompMgr* mgr, AcCompInfo* info, AcMsg* msg) {
  AC_UNUSED(mgr);

  AcDispatcher_send_msg(info->dc, msg);
  AcReceptor_signal(info->dtp->waiting);
}

/**
 * see ac_comp_mgr.h
 */
void AcCompMgr_deinit(AcCompMgr* mgr) {
  ac_debug_printf("AcCompMgr_deinit:+mgr=%p\n", mgr);
  if (mgr != AC_NULL) {
    for (ac_u32 i = 0; i < mgr->max_dtps; i++) {
      DispatchThreadParams* dtp = &mgr->dtps[i];

      if (dtp->thread_started) {
        // Stop the thread and kick it so it stops
        __atomic_store_n(&dtp->stop_processing_msgs, AC_TRUE, __ATOMIC_RELEASE);
        AcReceptor_signal(dtp->waiting);

        // Wait until the thread is done
        AcReceptor_wait(dtp->done);

        dtp->thread_started = AC_FALSE;
      }

      if (dtp->cis != AC_NULL) {
        ac_debug_printf("AcCompMgr_deinit: mgr=%p free dtp->cis=%p\n", mgr, dtp->cis);
        ac_free(dtp->cis);
        dtp->cis = AC_NULL;
      }

      AcReceptor_ret(dtp->done);
      AcReceptor_ret(dtp->ready);
    }

    // TODO: Shouldn't have to remove_zombies
#if AC_PLATFORM == pc_x86_64
    ac_thread_yield();
    remove_zombies();
#endif

    if (mgr->comp_infos != AC_NULL) {
      ac_debug_printf("AcCompMgr_deinit: mgr=%p free mgr->comp_infos=%p\n", mgr, mgr->comp_infos);
      ac_free(mgr->comp_infos);
      mgr->comp_infos = AC_NULL;
    }

    if (mgr->dtps != AC_NULL) {
      ac_debug_printf("AcCompMgr_deinit: mgr=%p free mgr->dtps=%p\n", mgr, mgr->dtps);
      ac_free(mgr->dtps);
      mgr->dtps = AC_NULL;
    }

    ac_debug_printf("AcCompMgr_deinit: free mgr=%p\n", mgr);
    ac_free(mgr);
  }
  ac_debug_printf("AcCompMgr_deinit:-mgr=%p\n", mgr);
}

/**
 * see ac_comp_mgr.h
 */
AcCompMgr* AcCompMgr_init(ac_u32 max_component_threads, ac_u32 max_components_per_thread,
    ac_u32 stack_size) {
  ac_bool error = AC_FALSE;
  AcCompMgr* mgr = ac_calloc(1, sizeof(AcCompMgr));
  mgr->dtps = AC_NULL;
  mgr->comp_infos = AC_NULL;
  mgr->max_dtps = max_component_threads;
  

  ac_debug_printf("AcCompMgr_init:+max_component_threads=%d max_components_per_thread=%d stack_size=%d\n",
      max_component_threads, max_components_per_thread, stack_size);
      
  if (max_component_threads == 0) {
    ac_printf("Counld not create the AcCompMgr max_component_threads is 0\n");
    error = AC_TRUE;
    goto done;
  }

  if (max_components_per_thread == 0) {
    ac_printf("Counld not create the AcCompMgr max_components_per_thread is 0\n");
    error = AC_TRUE;
    goto done;
  }

  mgr->dtps = ac_calloc(mgr->max_dtps, sizeof(DispatchThreadParams));
  if (mgr->dtps == AC_NULL) {
    error = AC_TRUE;
    goto done;
  }
  mgr->comp_infos_max_count = mgr->max_dtps * max_components_per_thread;
  mgr->comp_infos = ac_calloc(mgr->comp_infos_max_count, sizeof(AcCompInfo));
  if (mgr->comp_infos == AC_NULL) {
    error = AC_TRUE;
    goto done;
  }

  ac_debug_printf("AcCompMgr_init: loop\n");
  for (ac_u32 i = 0; i < mgr->max_dtps; i++) {
    DispatchThreadParams* dtp = &mgr->dtps[i];
    ac_debug_printf("AcCompMgr_init: dtps[%d]=%p\n", i, dtp);
    dtp->max_comps = max_components_per_thread;
    dtp->cis = ac_calloc(dtp->max_comps, sizeof(AcCompInfo*));
    if (dtp->cis == AC_NULL) {
      ac_debug_printf("AcCompMgr_init: i=%d\n dtp->cis == AC_NULL", i);
      error = AC_TRUE;
      goto done;
    }
    for (ac_u32 j = 0; j < dtp->max_comps; j++) {
      ac_u32 ci_idx = (i * dtp->max_comps) + j;
      AcCompInfo* ci = &mgr->comp_infos[ci_idx];
      ac_debug_printf("AcCompMgr_init: i=%d j=%d ci_idx=%d ci=%p\n", i, j, ci_idx, ci);
      dtp->cis[j] = ci;
      ci->comp_idx = ci_idx;
      ci->dtp = dtp;
      ci->comp = AC_NULL;
      ci->dc = AC_NULL;
    }
    dtp->done = AcReceptor_get();
    ac_assert(dtp->done != AC_NULL);
    dtp->ready = AcReceptor_get();
    ac_assert(dtp->ready != AC_NULL);

    ac_thread_rslt_t rslt = ac_thread_create(stack_size, dispatch_thread, dtp);
    dtp->thread_started = rslt.status == 0;
    if (!dtp->thread_started) {
      ac_printf("AcCompMgr_init: Counld not create the dispatch_thread %d rslt.status=%d\n",
          i, rslt.status);
      error = AC_TRUE;
      goto done;
    }

    // Wait until the thread is ready
    AcReceptor_wait(dtp->ready);
  }

  // Initialize the next dt to add a component too
  mgr->next_dtps = 0;

done:
  if (error) {
    AcCompMgr_deinit(mgr);
    mgr = AC_NULL;
  }
  return mgr;
}
