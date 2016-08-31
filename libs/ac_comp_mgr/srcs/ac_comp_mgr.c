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
#include <ac_memset.h>
#include <ac_msg.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_receptor.h>
#include <ac_status.h>
#include <ac_string.h>
#include <ac_thread.h>

#if AC_PLATFORM == pc_x86_64
extern void remove_zombies(void);
#endif

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

  // Create the waiting receptor and init our stop_processing_msgs flag
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
    AcComp** pcomp = params->comps[j];
    AcComp* comp = __atomic_load_n(pcomp, __ATOMIC_ACQUIRE);
    if ((comp != AC_NULL) &&
        __atomic_compare_exchange_n(pcomp, &comp, AC_NULL, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
      ac_debug_printf("disptach_thread: call AcDispatcher_rmv_comp(%s)\n", comp->name);
      AcDispatcher_rmv_comp(comp->ci.dtp->d, comp);
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
  ac_debug_printf("AcCompMgr_find_comp:+name=%s\n", name);
  AcComp* comp = AC_NULL;
  ac_bool found = AC_FALSE;

  // Search the list CompInfos for the name
  for (ac_u32 i = 0; i < mgr->comps_max_count; i++) {
    comp = mgr->comps[i];
    if (comp != AC_NULL) {
      if (ac_strncmp((const char*)name,
            (const char*)comp->name, ac_strlen((const char*)comp->name)) == 0) {
        ac_debug_printf("AcCompMgr_find_comp: name=%s, found\n", name);
        found = AC_TRUE;
        break;
      }
    }
  }

  if (!found) {
    comp = AC_NULL;
  }
  ac_debug_printf("AcCompMgr_find_comp:-name=%s comp=%p\n", name, comp);
  return comp;
}

/**
 * see ac_comp_mgr.h
 */
AcStatus AcCompMgr_add_comp(AcCompMgr* mgr, AcComp* comp) {
  AcStatus status = AC_STATUS_ERR;
  ac_bool found = AC_FALSE;

  for (ac_u32 thrd = 0; !found && (thrd < mgr->max_dtps); thrd++) {
    // Search the dtps
    ac_u32 idx = __atomic_fetch_add(&mgr->next_dtps, 1, __ATOMIC_RELEASE);
    idx %= mgr->max_dtps;

    DispatchThreadParams* dtp = &mgr->dtps[idx];
    ac_debug_printf("AcCompMgr_add_comp:+comp=%p idx=%d dtp=%p dtp->d=%p dtp->max_comps=%d\n",
        comp, idx, dtp, dtp->d, dtp->max_comps);

    // Search the list Comps for an unused slot,
    // i.e. mgr->comps[i] == AC_NULL
    for (ac_u32 i = 0; i < dtp->max_comps; i++) {
      AcComp** pcomp = dtp->comps[i];
      AcComp* null_comp = AC_NULL;

      if (__atomic_compare_exchange_n(pcomp, &null_comp, comp, AC_TRUE,
            __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {

        // Found an empty entry add the component to the dispatcher
        AcCompInfo* ci = &comp->ci;
        ci->dc = AcDispatcher_add_comp(dtp->d, comp);
        if (ci->dc == AC_NULL) {
          ac_debug_printf("AcCompMgr_add_comp: %i dtp->d=%p, could not add comp=%p\n",
              i, dtp->d, comp);
        } else {
          ac_debug_printf("AcCompMgr_add_comp: i=%d added *pcomp=%p comp=%s\n",
              i, *pcomp, comp->name);
          ci->mgr = mgr;
          ci->comp_idx = dtp->comp_idx;
          ci->dtp = dtp;
          status = AC_STATUS_OK;
          found = AC_TRUE;
          break;
        }
      }
    }
  }

  ac_debug_printf("AcCompMgr_add_comp:-comp=%p status=%u\n", comp, status);
  return status;
}

/**
 * see ac_comp_mgr.h
 */
AcStatus AcCompMgr_rmv_comp(AcComp* comp) {
  ac_debug_printf("AcCompMgr_rmv_comp:+comp=%s\n", comp->name);
  AcStatus status;
  if (comp == AC_NULL) {
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  AcCompInfo* ci = &comp->ci;

  AcComp** pcomp = &ci->mgr->comps[ci->comp_idx];
  if (__atomic_compare_exchange_n(pcomp, &comp, AC_NULL, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
    ac_debug_printf("AcCompMgr_rmv_comp: call AcDispatcher_rmv_comp(%s)\n", comp->name);
    AcDispatcher_rmv_comp(comp->ci.dtp->d, comp);
    ci->dc = AC_NULL;
    ci->mgr = AC_NULL;
    ci->comp_idx = 0;
    ci->dtp = AC_NULL;
  }

  status = AC_STATUS_OK;

done:
  ac_debug_printf("AcCompMgr_rmv_comp:-comp=%s ret status=%u\n", comp->name, status);
  return status;
}


/**
 * see ac_comp_mgr.h
 */
void AcCompMgr_send_msg(AcComp* comp, AcMsg* msg) {
  // TODO: Race with AcCompMgr_rmv_comp!!!!!
  AcDispatcher_send_msg(comp->ci.dc, msg);
  AcReceptor_signal(comp->ci.dtp->waiting);
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

      if (dtp->comps != AC_NULL) {
        ac_debug_printf("AcCompMgr_deinit: mgr=%p free dtp->comps=%p\n", mgr, dtp->comps);
        ac_free(dtp->comps);
        dtp->comps = AC_NULL;
      }

      AcReceptor_ret(dtp->done);
      AcReceptor_ret(dtp->ready);
    }

    // TODO: Shouldn't have to remove_zombies
#if AC_PLATFORM == pc_x86_64
    ac_thread_yield();
    remove_zombies();
#endif

    if (mgr->comps != AC_NULL) {
      ac_debug_printf("AcCompMgr_deinit: mgr=%p free mgr->comps=%p\n", mgr, mgr->comps);
      ac_free(mgr->comps);
      mgr->comps = AC_NULL;
    }

    if (mgr->dtps != AC_NULL) {
      ac_debug_printf("AcCompMgr_deinit: mgr=%p free mgr->dtps=%p\n", mgr, mgr->dtps);
      ac_free(mgr->dtps);
      mgr->dtps = AC_NULL;
    }
  }
  ac_debug_printf("AcCompMgr_deinit:-mgr=%p\n", mgr);
}

/**
 * see ac_comp_mgr.h
 */
AcStatus AcCompMgr_init(AcCompMgr* mgr, ac_u32 max_component_threads, ac_u32 max_components_per_thread,
    ac_u32 stack_size) {
  AcStatus status;

  ac_memset(mgr, 0, sizeof(AcCompMgr));
  mgr->dtps = AC_NULL;
  mgr->comps = AC_NULL;
  mgr->max_dtps = max_component_threads;
  

  ac_debug_printf("AcCompMgr_init:+max_component_threads=%d max_components_per_thread=%d stack_size=%d\n",
      max_component_threads, max_components_per_thread, stack_size);
      
  if (max_component_threads == 0) {
    ac_printf("Counld not create the AcCompMgr max_component_threads is 0\n");
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  if (max_components_per_thread == 0) {
    ac_printf("Counld not create the AcCompMgr max_components_per_thread is 0\n");
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }

  mgr->dtps = ac_calloc(mgr->max_dtps, sizeof(DispatchThreadParams));
  if (mgr->dtps == AC_NULL) {
    status = AC_STATUS_OUT_OF_MEMORY;
    goto done;
  }
  mgr->comps_max_count = mgr->max_dtps * max_components_per_thread;
  mgr->comps = ac_calloc(mgr->comps_max_count, sizeof(AcComp*));
  if (mgr->comps == AC_NULL) {
    ac_debug_printf("AcCompMgr_init: mgr->comps == AC_NULL");
    status = AC_STATUS_OUT_OF_MEMORY;
    goto done;
  }

  ac_debug_printf("AcCompMgr_init: loop\n");
  for (ac_u32 i = 0; i < mgr->max_dtps; i++) {
    DispatchThreadParams* dtp = &mgr->dtps[i];
    ac_debug_printf("AcCompMgr_init: dtps[%d]=%p\n", i, dtp);
    dtp->max_comps = max_components_per_thread;
    dtp->comps = ac_calloc(dtp->max_comps, sizeof(AcComp*));
    if (dtp->comps == AC_NULL) {
      ac_debug_printf("AcCompMgr_init: i=%d\n dtp->cis == AC_NULL", i);
      status = AC_STATUS_ERR;
      goto done;
    }
    for (ac_u32 j = 0; j < dtp->max_comps; j++) {
      ac_u32 comp_idx = (i * dtp->max_comps) + j;
      AcComp** pcomp = &mgr->comps[comp_idx];
      ac_debug_printf("AcCompMgr_init: i=%d j=%d comps_idx=%d pcomp=%p\n", i, j, comp_idx, pcomp);
      dtp->comps[j] = pcomp;
      dtp->comp_idx = comp_idx;
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
      status = AC_STATUS_ERR;
      goto done;
    }

    // Wait until the thread is ready
    AcReceptor_wait(dtp->ready);
  }

  // Initialize the next dt to add a component too
  mgr->next_dtps = 0;

  status = AC_STATUS_OK;

done:
  if (status != AC_STATUS_OK) {
    AcCompMgr_deinit(mgr);
  }
  return status;
}
