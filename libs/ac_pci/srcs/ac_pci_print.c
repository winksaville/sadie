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

#include <ac_pci_print.h>

#include <ac_printf.h>

/**
 * Get PCI vendor Id
 *
 * @return 0xFFFF if no vendor id
 */
void ac_pci_cfg_addr_print(char* str, ac_pci_cfg_addr addr, char* terminator) {
  if (str != AC_NULL) {
    ac_printf("%s", str);
  }
  ac_printf("enable=%d resv=%d bus=%d dev=%d func=%d reg=%d",
      addr.enable, addr.resv, addr.bus, addr.dev, addr.func, addr.reg);
  if (terminator != AC_NULL) {
    ac_printf("%s", terminator);
  }
}

void ac_pci_cfg_common_hdr_print(char* indent_str, ac_pci_cfg_common_hdr* header) {
  if (indent_str == AC_NULL) {
    indent_str = "";
  }
  ac_printf("%svendor_id=0x%x\n", indent_str, header->vendor_id);
  ac_printf("%sdevice_id=0x%x\n", indent_str, header->device_id);
  ac_printf("%scommand=0x%x\n", indent_str, header->command);
  ac_printf("%sstatus=0x%x\n", indent_str, header->status);
  ac_printf("%srevision=0x%x\n", indent_str, header->revision_id);
  ac_printf("%sprog_if=0x%x\n", indent_str, header->prog_if);
  ac_printf("%ssub_class=0x%x\n", indent_str, header->sub_class);
  ac_printf("%sbase_class=0x%x\n", indent_str, header->base_class);
  ac_printf("%scache_line_size=0x%x\n", indent_str, header->cache_line_size);
  ac_printf("%slatency_timer=0x%x\n", indent_str, header->latency_timer);
  ac_printf("%sheader_type=0x%x\n", indent_str, header->header_type);
  ac_printf("%sbist=0x%x\n", indent_str, header->bist);
}

void ac_pci_cfg_hdr0_print(char* indent_str, ac_pci_cfg_hdr0* header) {
  if (indent_str == AC_NULL) {
    indent_str = "";
  }
  ac_pci_cfg_common_hdr_print(indent_str, &header->common_hdr);
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(header->base_addrs); i++) {
    ac_printf("%sbase_addrs[%d]=0x%x\n", indent_str, i, header->base_addrs[i]);
  }
  ac_printf("%scardbus_cis_ptr=0x%x\n", indent_str, header->cardbus_cis_ptr);
  ac_printf("%ssubsystem_vendor_id=0x%x\n", indent_str, header->subsystem_vendor_id);
  ac_printf("%ssubsystem_id=0x%x\n", indent_str, header->subsystem_id);
  ac_printf("%srom_base_addr=0x%x\n", indent_str, header->rom_base_addr);
  ac_printf("%scapabilities=0x%x\n", indent_str, header->capabilities);
  ac_printf("%sinterrupt_line=0x%x\n", indent_str, header->interrupt_line);
  ac_printf("%sinterrupt_pin=0x%x\n", indent_str, header->interrupt_pin);
  ac_printf("%smin_grant=0x%x\n", indent_str, header->min_grant);
  ac_printf("%smax_grant=0x%x\n", indent_str, header->max_grant);
}

void ac_pci_cfg_hdr1_print(char* indent_str, ac_pci_cfg_hdr1* header) {
  if (indent_str == AC_NULL) {
    indent_str = "";
  }
  ac_pci_cfg_common_hdr_print(indent_str, &header->common_hdr);
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(header->base_addrs); i++) {
    ac_printf("%sbase_addrs[%d]=0x%x\n", indent_str, i, header->base_addrs[i]);
  }
  ac_printf("%sprimary_bus_number=0x%x\n", indent_str, header->primary_bus_number);
  ac_printf("%ssecondary_bus_number=0x%x\n", indent_str, header->secondary_bus_number);
  ac_printf("%ssubordinate_bus_number=0x%x\n", indent_str, header->subordinate_bus_number);
  ac_printf("%ssecondary_latency_timer=0x%x\n", indent_str, header->secondary_latency_timer);
  ac_printf("%sio_base=0x%x\n", indent_str, header->io_base);
  ac_printf("%sio_limit=0x%x\n", indent_str, header->io_limit);
  ac_printf("%sprefetchable_memory_base=0x%x\n", indent_str, header->prefetchable_memory_base);
  ac_printf("%sprefetchable_memory_limit=0x%x\n", indent_str, header->prefetchable_memory_limit);
  ac_printf("%sprefetchable_hi32_memory_base=0x%x\n", indent_str, header->prefetchable_hi32_memory_base);
  ac_printf("%sprefetchable_hi32_memory_limit=0x%x\n", indent_str, header->prefetchable_hi32_memory_limit);
  ac_printf("%sio_hi16_base=0x%x\n", indent_str, header->io_hi16_base);
  ac_printf("%sio_hi16_limit=0x%x\n", indent_str, header->io_hi16_limit);
  ac_printf("%scapabilities=0x%x\n", indent_str, header->capabilities);
  ac_printf("%sexpansion_rom_base=0x%x\n", indent_str, header->expansion_rom_base);
  ac_printf("%sinterrupt_line=0x%x\n", indent_str, header->interrupt_line);
  ac_printf("%sinterrupt_pin=0x%x\n", indent_str, header->interrupt_pin);
  ac_printf("%sbridge_control=0x%x\n", indent_str, header->bridge_control);
}

void ac_pci_cfg_hdr_print(char* indent_str, ac_pci_cfg_hdr* header) {
  if (indent_str == AC_NULL) {
    indent_str = "";
  }
  switch (header->hdr0.common_hdr.header_type & 0x7f) {
    case 0: {
      ac_pci_cfg_hdr0_print(indent_str, &header->hdr0);
      break;
    }
    case 1: {
      ac_pci_cfg_hdr1_print(indent_str, &header->hdr1);
      break;
    }
    default: {
      ac_printf("header_type %d is NOT supported\n",
          header->hdr0.common_hdr.header_type);
      break;
    }
  }
}
