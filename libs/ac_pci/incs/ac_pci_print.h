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

#ifndef SADIE_LIBS_AC_PCI_INCS_AC_PCI_PRINT_H
#define SADIE_LIBS_AC_PCI_INCS_AC_PCI_PRINT_H

#include <ac_pci.h>

/**
 * Print the ac_pci_cfg_addr
 */
void ac_pci_cfg_addr_print(char* str, ac_pci_cfg_addr addr, char* terminator);

/**
 * Print PCI common header
 */
void ac_pci_cfg_hdr_cmn_print(char* indent_str, ac_pci_cfg_hdr_cmn* header);

/**
 * Print PCI header type0
 */
void ac_pci_cfg_hdr0_print(char* indent_str, ac_pci_cfg_hdr0* header);

/**
 * Print PCI header type1
 */
void ac_pci_cfg_hdr1_print(char* indent_str, ac_pci_cfg_hdr1* header);

/**
 * Print PCI header
 */
void ac_pci_cfg_hdr_print(char* indent_str, ac_pci_cfg_hdr* header);

#endif
