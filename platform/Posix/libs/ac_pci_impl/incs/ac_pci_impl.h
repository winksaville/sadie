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

#ifndef SADIE_PLATFORM_POSIX_LIBS_AC_PCI_IMPL_INCS_AC_PCI_IMPL_H
#define SADIE_PLATFORM_POSIX_LIBS_AC_PCI_IMPL_INCS_AC_PCI_IMPL_H

#include <ac_inttypes.h>

/**
 * pci configuration read u32.
 *
 * @return 0xFFFFFFFF if that address doesn't exist.
 */
static inline ac_u32 ac_pci_cfg_rd_u32(ac_pci_cfg_addr cfg_addr) {
  return 0xffffffff;
}

/**
 * pci configuration write u32
 */
static inline void ac_pci_cfg_wr_u32(ac_pci_cfg_addr cfg_addr, ac_u32 val) {
  // NOT Implemented
}

#endif
