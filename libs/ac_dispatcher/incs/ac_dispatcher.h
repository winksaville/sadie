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

#ifndef SADIE_LIBS_AC_DISPATCHER_H
#define SADIE_LIBS_AC_DISPATCHER_H

#include <ac_comp_mgr.h>
#include <ac_msg.h>

typedef struct AcDispatchableComp AcDispatchableComp;

// The opaque ac_dipatcher
typedef struct AcDispatcher AcDispatcher;

/**
 * Dispatch messages to asynchronous components
 */
ac_bool AcDispatcher_dispatch(AcDispatcher* d);

/**
 * Get a dispatcher able to support max_count AcComp's.
 */
AcDispatcher* AcDispatcher_get(ac_u32 max_count);

/**
 * Return dispatcher to the system.
 */
void AcDispatcher_ret(AcDispatcher* d);

/**
 * Add the AcComp to this dispatcher
 *
 * @return: AcDispatableComp* or AC_NULL if an error,
 * this will occur if there are to many AcComp's registered.
 */
AcDispatchableComp* AcDispatcher_add_comp(AcDispatcher* d, AcComp* comp);

/**
 * Remove all instances associated with the dispatchable component
 *
 * return the AcComp or AC_NULL if the associated AcComp was not found.
 */
AcComp* AcDispatcher_rmv_comp(AcDispatcher* d, AcDispatchableComp* comp);

/**
 * Send a message to dispatchable component
 *
 * @param: dc is the dispatchable component previously added.
 * @param: msg is the message to send
 */
void AcDispatcher_send_msg(AcDispatchableComp* dc, AcMsg* msg);

#endif
