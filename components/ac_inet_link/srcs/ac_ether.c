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

#include <ac_ether.h>

#include <ac_inet_hton.h>

#include <ac_memcpy.h>
#include <ac_inttypes.h>

/**
 * Initialize AcEtherHdr
 */
void ac_ether_init(
    AcEtherHdr* pEthHdr,            ///< Ether header to initialize
    const void *dst_addr,           ///< Destination address
    const void* src_addr,           ///< Source address
    AcU16 protocol) {               ///< protocol of the contents
  ac_memcpy(pEthHdr->dst, dst_addr, AC_ETHER_ADDR_LEN);
  ac_memcpy(pEthHdr->src, src_addr, AC_ETHER_ADDR_LEN);
  pEthHdr->proto = AC_HTON_U16(protocol);
}
