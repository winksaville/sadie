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

#ifndef LIBS_AC_MSG_POOL_TESTS_INCS_TEST_H
#define LIBS_AC_MSG_POOL_TESTS_INCS_TEST_H

#include <ac_inttypes.h>
#include <ac_msg_pool.h>

/**
 * Test we can create, send a message and remove components.
 *
 * @param: cm is AcCompMgr to use
 * @param: mp is AcMsgPool to use
 * @param: comp_count is number of components to create
 *
 * @return: AC_TRUE if an error
 */
ac_bool test_comps(AcCompMgr* cm, AcMsgPool* mp, ac_u32 comp_count);

#endif
