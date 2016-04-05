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

#ifndef SADIE_ARCH_X86_LIBS_AC_PCI_IMPL_INCS_AC_PCI_IMPL_H
#define SADIE_ARCH_X86_LIBS_AC_PCI_IMPL_INCS_AC_PCI_IMPL_H

#include <io_x86.h>

#include <ac_bits.h>
#include <ac_inttypes.h>

#define __PCI_CFG_ADDR_PORT 0xcf8
#define __PCI_CFG_DATA_PORT 0xcfc

/**
 * pci configuration read u32.
 *
 * @return 0xFFFFFFFF if that address doesn't exist.
 */
static inline ac_u32 ac_pci_cfg_rd_u32(ac_pci_cfg_addr cfg_addr) {
  cfg_addr.reg &= 0xfffffffc;
  union ac_pci_cfg_addr_u addr_u = { .fields = cfg_addr };
  io_wait();
  io_wr_u32(__PCI_CFG_ADDR_PORT, addr_u.raw);
  io_wait();
  return io_rd_u32(__PCI_CFG_DATA_PORT);
}

/**
 * pci configuration write u32
 */
static inline void ac_pci_cfg_wr_u32(ac_pci_cfg_addr cfg_addr, ac_u32 val) {
  union ac_pci_cfg_addr_u addr_u = { .fields = cfg_addr };
  io_wait();
  io_wr_u32(__PCI_CFG_ADDR_PORT, addr_u.raw);
  io_wait();
  io_wr_u32(__PCI_CFG_DATA_PORT, val);
}

#endif
