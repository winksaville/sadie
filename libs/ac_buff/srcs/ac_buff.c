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
 * Debug code for AcBuff
 */

#include <ac_buff.h>

#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_status.h>
#include <ac_printf.h>

/**
 * Free prviously allocated AcBuff's
 */
void AcBuff_free(AcBuff* buffs) {
  ac_free(buffs);
}

/**
 * Return a buff to its pool
 *
 * @params buff is an AcBuff
 */
void AcBuff_ret(AcBuff* buff) {
  ac_printf("AcBuff_ret: buff=%p pool_fifo=%p\n", buff, buff->hdr.pool_fifo);
  if (buff != AC_NULL) {
    if (buff->hdr.pool_fifo != AC_NULL) {
      AcMpscFifo_add_ac_buff(buff->hdr.pool_fifo, buff);
    } else {
      AcBuff_free(buff);
    }
  }
}

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
AcStatus AcBuff_alloc(AcMpscFifo* fifo, ac_u32 count,
    ac_u32 data_size, ac_u32 user_size, AcBuff** buffs) {
  AcStatus status;
  AcBuff* buff_array = AC_NULL;

  if ((count == 0) || (data_size == 0) || (data_size < user_size)) {
    status = AC_STATUS_BAD_PARAM;
    *buffs = AC_NULL;
    goto done;
  }

  ac_u32 alloc_size = sizeof(AcBuff) + data_size;
  ac_printf("alloc_size=%d\n", alloc_size);
  buff_array = ac_calloc(count, alloc_size);
  for (ac_u32 i = 0; i < count; i++) {
    AcBuff* buff = AcBuff_get_nth(buff_array, i, data_size);
    buff->hdr.next = AC_NULL;
    buff->hdr.pool_fifo = fifo;
    buff->hdr.data_size = data_size;
    buff->hdr.user_size = user_size;
  }
  *buffs = buff_array;

  status = AC_STATUS_OK;
done:
  if (status != AC_STATUS_OK) {
    ac_free(buff_array);
  }

  return status;
}
