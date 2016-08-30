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

#include <ac_inttypes.h>

typedef struct DispatchThreadParams DispatchThreadParams;
typedef struct AcCompInfo AcCompInfo;

/**
 * A component manager as returned by AcCompMgr_init
 */
typedef struct AcCompMgr {
  AcCompInfo* comp_infos;     // Array of AcCompInfo objects being managed
                              // across all of the threads
  AcU32 comp_infos_max_count; // Number of elements in comp_infos array
  DispatchThreadParams* dtps; // Array of DispathThreadParams, one for each thread
  AcU32 max_dtps;             // Number of threads in the dtps array
  AcU32 next_dtps;            // Next thread
} AcCompMgr;

#endif

