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

#ifndef SADIE_LIBS_INCS_AC_INET_H
#define SADIE_LIBS_INCS_AC_INET_H

#include <ac_inttypes.h>
#include <ac_assert.h>

#define AC_IPV4_ADDR_LEN 4
#define AC_IPV6_ADDR_LEN 16

typedef struct AC_ATTR_PACKED {
  union {
    AcU8  ary_u8[AC_IPV6_ADDR_LEN];
    AcU16 ary_u16[AC_IPV6_ADDR_LEN/sizeof(AcU16)];
    AcU32 ary_u32[AC_IPV6_ADDR_LEN/sizeof(AcU32)];
    AcU64 ary_u64[AC_IPV6_ADDR_LEN/sizeof(AcU64)];
  };
} AcIpv6Addr;

ac_static_assert(sizeof(AcIpv6Addr) == AC_IPV6_ADDR_LEN, L"sizeof(AcIpv6Addr) != 16");

#endif
