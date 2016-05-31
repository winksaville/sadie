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
#include <ac_memmgr.h>
#include <ac_mpscfifo.h>
#include <ac_msg.h>
#include <ac_debug_printf.h>

/**
 * A opaque ID for an AcComp
 */
typedef struct AcCompId {
  AcComp* comp;
} AcCompId;

/**
 * Initialize the component manager, may only be called once.
 *
 * @param: max_component_threads is the maximum number of threads to manage
 * @param: max_components_thread is the maximum number of components per thread
 * @param: stack_size is number of bytes for a threads stack, 0 will provide the default
 */
void AcCompMgr_init(ac_u32 max_component_threads, ac_u32 max_components_per_thread,
    ac_u32 stack_size) {
}

/**
 * Add a component to be managed
 *
 * @param: comp is an initialized component type
 *
 * @return: AcCompId or AC_NULL if an error
 */
AcCompId* AcCompMgr_add_comp(AcComp* comp) {
  return AC_NULL;
}

/**
 * Remove a component being managed
 *
 * @param: comp is an initialized component type
 */
void AcCompMgr_rmv_comp(AcCompId* id) {
}
