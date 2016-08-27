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
 * Debug code for ac_mpsc_link_list
 */

#ifndef SADIE_LIBS_AC_MPSC_LINK_LIST_AC_MPSC_LINK_LIST_DBG_H
#define SADIE_LIBS_AC_MPSC_LINK_LIST_AC_MPSC_LINK_LIST_DBG_H

#include <ac_mpsc_link_list.h>

/**
 * Print a AcNextPtr
 */
void AcNextPtr_print(const char* leader, AcNextPtr* next);

#ifdef NDEBUG
  #define AcNextPtr_debug_print(leader, list) ((void)(0))
#else
  #define AcNextPtr_debug_print(leader, list) AcNextPtr_print(leader, list)
#endif

/**
 * Print a AcMpscLinkList
 */
void AcMpscLinkList_print(const char* leader, AcMpscLinkList* list);

#ifdef NDEBUG
  #define AcMpscLinkList_debug_print(leader, list) ((void)(0))
#else
  #define AcMpscLinkList_debug_print(leader, list) AcMpscLinkList_print(leader, list)
#endif

#endif
