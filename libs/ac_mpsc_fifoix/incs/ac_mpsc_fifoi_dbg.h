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
 * Debug code for mpscfifoi
 */

#ifndef SADIE_LIBS_AC_MPSC_FIFOI_AC_MPSC_FIFOI_DBG_H
#define SADIE_LIBS_AC_MPSC_FIFOI_AC_MPSC_FIFOI_DBG_H

#include <ac_mpsc_fifo.h>

/**
 * Print a AcMpscFifo
 */
void AcMpscFifoI_print(const char* leader, AcMpscFifoI* fifo);

#ifdef NDEBUG
  #define AcMpscFifoI_debug_print(leader, fifo) ((void)(0))
#else
  #define AcMpscFifoI_debug_print(leader, fifo) AcMpscFifoI_print(leader, fifo)
#endif

#endif
