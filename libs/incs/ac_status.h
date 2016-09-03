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

#ifndef SADIE_LIBS_INCS_AC_STATUS_H
#define SADIE_LIBS_INCS_AC_STATUS_H

#include <ac_inttypes.h>
#include <ac_bits.h>

typedef ac_u32 AcStatus;

#define AC_STATUS(major, minor) ( \
  AC_SET_BITS(AcStatus, 0u, (major), 16, 16) \
  | AC_SET_BITS(AcStatus, 0u, (minor), 16, 0) )

#define AC_STATUS_MAJOR(status) AC_GET_BITS(AcStatus, (status), 16, 16)
#define AC_STATUS_MINOR(status) AC_GET_BITS(AcStatus, (status), 16, 0)

/**
 * AcStatus values
 */
#define AC_STATUS_OK                            AC_STATUS(0, 0)
#define AC_STATUS_ERR                           AC_STATUS(1, 0)
#define AC_STATUS_BAD_PARAM                     AC_STATUS(2, 0)
#define AC_STATUS_OUT_OF_MEMORY                 AC_STATUS(3, 0)
#define AC_STATUS_NOT_AVAILABLE                 AC_STATUS(4, 0)
#define AC_STATUS_UNRECOGNIZED_PROTOCOL         AC_STATUS(5, 0)
#define AC_STATUS_UNRECOGNIZED_OPERATION        AC_STATUS(6, 0)
#define AC_STATUS_LINUX_ERR(errno)              AC_STATUS(7, (errno))

#endif
