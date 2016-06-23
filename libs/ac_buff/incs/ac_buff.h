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

#ifndef SADIE_LIBS_AC_BUFF_INCS_AC_BUFF_H
#define SADIE_LIBS_AC_BUFF_INCS_AC_BUFF_H

#include <ac_buff.h>
#include <ac_inttypes.h>
#include <ac_status.h>

typedef struct AcMpscFifo AcMpscFifo;
typedef struct AcBuff AcBuff;

typedef struct __attribute__((packed)) AcBuffHdr {
  AcBuff* next;         // Next AcBuff
  AcMpscFifo* fifo;     // Fifo this AcBuff is allocated from
  ac_u32 data_size;     // Size of the data array following this header
  ac_u32 user_size;     // Size of the user area in data array
} AcBuffHdr;

typedef struct __attribute__((packed)) AcBuff {
  AcBuffHdr hdr;        // The header
  ac_u8 data[];         // The buffers data
} AcBuff;

#define AcBuff_get_nth(array, index, data_size) ({ \
    AcBuff* nth = array; \
    nth += (index  * (sizeof(AcBuff) + data_size)); \
})

/**
 * Free prviously allocated AcBuff's
 */
void AcBuff_free(AcBuff* buffs);

/**
 * Allocate one for more AcBuff contigiously
 *
 * @params fifo is the fifo this AcBuff belongs to
 * @params count is number of buffers to allocate, 0 set buffs to AC_NULL
 * @params data_size is size of each buffer
 * @params user_size is size available to user
 *
 * @return 0 (AC_STATUS_OK) if successful and if count > 0 then buff != AC_NULL
 */
AcStatus AcBuff_alloc(AcMpscFifo* fifo, ac_u32 count, ac_u32 data_size, ac_u32 user_size,
    AcBuff** buffs);

#endif
