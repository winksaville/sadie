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

void ac_pci_cfg_common_hdr_print(ac_pci_cfg_common_hdr* header) {
  ac_printf("vendor_id=0x%x\n", header->vendor_id);
  ac_printf("device_id=0x%x\n", header->device_id);
  ac_printf("command=0x%x\n", header->command);
  ac_printf("status=0x%x\n", header->status);
  ac_printf("revision=0x%x\n", header->revision_id);
  ac_printf("prog_if=0x%x\n", header->prog_if);
  ac_printf("sub_class=0x%x\n", header->sub_class);
  ac_printf("base_class=0x%x\n", header->base_class);
  ac_printf("cache_line_size=0x%x\n", header->cache_line_size);
  ac_printf("latency_timer=0x%x\n", header->latency_timer);
  ac_printf("header_type=0x%x\n", header->header_type);
  ac_printf("bist=0x%x\n", header->bist);
}

void ac_pci_cfg_hdr0_print(ac_pci_cfg_hdr0* header) {
  ac_pci_cfg_common_hdr_print(&header->common_hdr);
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(header->base_addrs); i++) {
    ac_printf("base_addrs[%d]=0x%x\n", i, header->base_addrs[i]);
  }
  ac_printf("cardbus_cis_ptr=0x%x\n", header->cardbus_cis_ptr);
  ac_printf("subsystem_vendor_id=0x%x\n", header->subsystem_vendor_id);
  ac_printf("subsystem_id=0x%x\n", header->subsystem_id);
  ac_printf("rom_base_addr=0x%x\n", header->rom_base_addr);
  ac_printf("capabilities=0x%x\n", header->capabilities);
  ac_printf("interrupt_line=0x%x\n", header->interrupt_line);
  ac_printf("interrupt_pin=0x%x\n", header->interrupt_pin);
  ac_printf("min_grant=0x%x\n", header->min_grant);
  ac_printf("max_grant=0x%x\n", header->max_grant);
}

void ac_pci_cfg_hdr1_print(ac_pci_cfg_hdr1* header) {
  ac_pci_cfg_common_hdr_print(&header->common_hdr);
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(header->base_addrs); i++) {
    ac_printf("base_addrs[%d]=0x%x\n", i, header->base_addrs[i]);
  }
  ac_printf("primary_bus_number=0x%x\n", header->primary_bus_number);
  ac_printf("secondary_bus_number=0x%x\n",
      header->secondary_bus_number);
  ac_printf("subordinate_bus_number=0x%x\n",
      header->subordinate_bus_number);
  ac_printf("secondary_latency_timer=0x%x\n",
      header->secondary_latency_timer);
  ac_printf("io_base=0x%x\n", header->io_base);
  ac_printf("io_limit=0x%x\n", header->io_limit);
  ac_printf("prefetchable_memory_base=0x%x\n",
      header->prefetchable_memory_base);
  ac_printf("prefetchable_memory_limit=0x%x\n",
      header->prefetchable_memory_limit);
  ac_printf("prefetchable_hi32_memory_base=0x%x\n",
      header->prefetchable_hi32_memory_base);
  ac_printf("prefetchable_hi32_memory_limit=0x%x\n",
      header->prefetchable_hi32_memory_limit);
  ac_printf("io_hi16_base=0x%x\n", header->io_hi16_base);
  ac_printf("io_hi16_limit=0x%x\n", header->io_hi16_limit);
  ac_printf("capabilities=0x%x\n", header->capabilities);
  ac_printf("expansion_rom_base=0x%x\n", header->expansion_rom_base);
  ac_printf("interrupt_line=0x%x\n", header->interrupt_line);
  ac_printf("interrupt_pin=0x%x\n", header->interrupt_pin);
  ac_printf("bridge_control=0x%x\n", header->bridge_control);
}

void ac_pci_cfg_hdr_print(char* str, ac_pci_cfg_hdr* header) {
  if (str != AC_NULL) {
    ac_printf("%s", str);
  }
  switch (header->hdr0.common_hdr.header_type) {
    case 0: {
      ac_pci_cfg_hdr0_print(&header->hdr0);
      break;
    }
    case 1: {
      ac_pci_cfg_hdr1_print(&header->hdr1);
      break;
    }
    default: {
      ac_printf("header_type %d is NOT supported\n",
          header->hdr0.common_hdr.header_type);
      break;
    }
  }
}
