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
 * A ac_msg_pool contains msgs which can be used for sending
 * information between components.
 */

#ifndef SADIE_LIBS_AC_COMP_MGR_INCS_AC_COMP_MGR_INTERNAL_H
#define SADIE_LIBS_AC_COMP_MGR_INCS_AC_COMP_MGR_INTERNAL_H

#include <ac_dispatcher.h>
#include <ac_inttypes.h>
#include <ac_receptor.h>
#include <ac_thread.h>

typedef struct DispatchThreadParams DispatchThreadParams;
typedef struct AcCompMgr AcCompMgr;
typedef struct AcComp AcComp;

/**
 * A opaque component info for an AcComp
 */
typedef struct AcCompInfo {
  AcCompMgr* mgr;
  //AcComp* comp;
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
  AcComp*** comps;
  ac_u32 comp_idx;
  AcReceptor* done;
  AcReceptor* ready;
  AcReceptor* waiting;
  ac_bool stop_processing_msgs;
} DispatchThreadParams;

/**
 * A component manager as returned by AcCompMgr_init
 */
typedef struct AcCompMgr {
  AcComp** comps;            // Array of AcComp pointer objects being managed
                              // across all of the threads
  AcU32 comps_max_count;      // Number of elements in comps array
  DispatchThreadParams* dtps; // Array of DispathThreadParams, one for each thread
  AcU32 max_dtps;             // Number of threads in the dtps array
  AcU32 next_dtps;            // Next thread
} AcCompMgr;

#endif
