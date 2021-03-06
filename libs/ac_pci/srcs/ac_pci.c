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
  ac_pci_cfg_hdr_cmn hdr_cmn;
  addr.reg = 0;
  hdr_cmn.raw[0] = ac_pci_cfg_rd_u32(addr);
  return hdr_cmn.vendor_id;
}

/**
 * Get PCI header type
 *
 * @return 0x7F if no header type
 */
ac_u8 ac_pci_cfg_get_hdr_type(ac_pci_cfg_addr addr) {
  ac_pci_cfg_hdr_cmn hdr_cmn;
  addr.reg = 12;
  hdr_cmn.raw[3] = ac_pci_cfg_rd_u32(addr);
  return hdr_cmn.hdr_type;
}

/**
 * Get PCI multi_func
 */
ac_bool ac_pci_cfg_get_multi_func(ac_pci_cfg_addr addr) {
  ac_pci_cfg_hdr_cmn hdr_cmn;
  addr.reg = 12;
  hdr_cmn.raw[3] = ac_pci_cfg_rd_u32(addr);
  return hdr_cmn.multi_func;
}

/**
 * Get PCI configuration header
 *
 * @return 0 if successful and header is retrieved
 */
ac_uint ac_pci_cfg_hdr_get(ac_pci_cfg_addr addr, ac_pci_cfg_hdr* header) {
  addr.reg = 0;
  header->hdr_cmn.raw[0] = ac_pci_cfg_rd_u32(addr);
  if (header->hdr_cmn.vendor_id != 0xFFFF) {
    for (int i = 1; i < AC_ARRAY_COUNT(header->hdr_cmn.raw); i++) {
      addr.reg = i * 4;
      header->hdr_cmn.raw[i] = ac_pci_cfg_rd_u32(addr);
    }
    if (header->hdr_cmn.hdr_type <= 1) {
      for (int i = 0; i < AC_ARRAY_COUNT(header->hdr0.raw); i++) {
        addr.reg = (AC_ARRAY_COUNT(header->hdr_cmn.raw) + i) * 4;
        header->hdr0.raw[i] = ac_pci_cfg_rd_u32(addr);
      }
      return 0;
    } else {
      return 1;
    }
  } else {
    return 1;
  }
}
