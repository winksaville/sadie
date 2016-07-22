/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#ifndef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_NTOH_H
#define SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_NTOH_H

#include <ac_inttypes.h>
#include <ac_swap_bytes.h>

#if AC_ARCH_BYTE_ORDER == AC_ARCH_BYTE_ORDER_LITTLE_ENDIAN

/* Swap bytes for BIG_ENDIAN machines */

#define AC_NTOH_U16(val) ({ \
  ac_u16 result = AC_SWAP_BYTES_U16(val); \
  result; \
})

#define AC_NTOH_U32(val) ({ \
  ac_u16 result = AC_SWAP_BYTES_U32(val); \
  result; \
})

#define AC_NTOH_U64(val) ({ \
  ac_u16 result = AC_SWAP_BYTES_U64(val); \
  result; \
})

#elif AC_ARCH_BYTE_ORDER == AC_ARCH_BYTE_ORDER_BIG_ENDIAN

/* Do nothing for BIG_ENDIAN machines */

#define AC_NTOH_U16(val) ({ \
  ac_u16 result = val; \
  result; \
})

#define AC_NTOH_U32(val) ({ \
  ac_u16 result = val; \
  result; \
})

#define AC_NTOH_U64(val) ({ \
  ac_u16 result = val; \
  result; \
})

#else

/* Whoops unknown AC_ARCH_BYTE_ORDER */
#error "ac_inet_ntoh.h: AC_ARCH_BYTE_ORDER is not little or big endian"

#endif

#endif
