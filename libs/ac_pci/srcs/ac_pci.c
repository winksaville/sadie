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

#include <ac_pci.h>
#include <ac_pci_print.h>

#include <ac_inttypes.h>

/**
 * Get PCI vendor Id
 *
 * @return 0xFFFF if no vendor id
 */
ac_u16 ac_pci_cfg_get_vendor_id(ac_pci_cfg_addr addr) {
  addr.reg = 0;
  return ac_pci_cfg_rd_u16(addr);
}

/**
 * Get PCI header type
 *
 * @return 0xFF if no header type
 */
ac_u8 ac_pci_cfg_get_header_type(ac_pci_cfg_addr addr) {
  addr.reg = 14;
  return ac_pci_cfg_rd_u8(addr);
}

/**
 * Get PCI configuration header
 *
 * @return 0 if successful and header is retrieved
 */
ac_uint ac_pci_cfg_hdr_get(ac_pci_cfg_addr addr, ac_pci_cfg_hdr* header) {

  if (ac_pci_cfg_get_vendor_id(addr) != 0xFFFF) {
#if 1
    for (int i = 0; i < AC_ARRAY_COUNT(header->raw_u32s); i++) {
      addr.reg = i * 4;
      header->raw_u32s[i] = ac_pci_cfg_rd_u32(addr);
    }
#else
    addr.reg = 0;
    header->hdr0.common_hdr.vendor_id = ac_pci_cfg_rd_u16(addr);
    addr.reg = 2;
    header->hdr0.common_hdr.device_id = ac_pci_cfg_rd_u16(addr);
    addr.reg = 6;
    header->hdr0.common_hdr.status = ac_pci_cfg_rd_u16(addr);
    addr.reg = 8;
    header->hdr0.common_hdr.revision_id = ac_pci_cfg_rd_u8(addr);
    addr.reg = 9;
    header->hdr0.common_hdr.prog_if = ac_pci_cfg_rd_u8(addr);
    addr.reg = 10;
    header->hdr0.common_hdr.sub_class = ac_pci_cfg_rd_u8(addr);
    addr.reg = 11;
    header->hdr0.common_hdr.base_class = ac_pci_cfg_rd_u8(addr);
    addr.reg = 12;
    header->hdr0.common_hdr.cache_line_size = ac_pci_cfg_rd_u8(addr);
    addr.reg = 13;
    header->hdr0.common_hdr.latency_timer = ac_pci_cfg_rd_u8(addr);
    addr.reg = 14;
    header->hdr0.common_hdr.header_type = ac_pci_cfg_rd_u8(addr);
    addr.reg = 15;
    header->hdr0.common_hdr.bist = ac_pci_cfg_rd_u8(addr);
#endif
    return 0;
  } else {
    return 1;
  }
}
