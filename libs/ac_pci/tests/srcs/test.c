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

#include <ac_printf.h>
#include <ac_test.h>

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
struct test_case {
  union ac_pci_cfg_addr_u val;
  ac_u32  reg;
  ac_u32  func;
  ac_u32  dev;
  ac_u32  bus;
  ac_u32  resv;
  ac_u32  enable;
};

static struct test_case test_case_array[] = {
  { .val.raw=0x00000001, .reg=0x1, },
  { .val.raw=0x00000080, .reg=0x80, },
  { .val.raw=0x00000100, .func=0x1, },
  { .val.raw=0x00000400, .func=0x4, },
  { .val.raw=0x00000800, .dev=0x1, },
  { .val.raw=0x00008000, .dev=0x10, },
  { .val.raw=0x00010000, .bus=0x01, },
  { .val.raw=0x00800000, .bus=0x80, },
  { .val.raw=0x01000000, .resv=0x01, },
  { .val.raw=0x40000000, .resv=0x40, },
  { .val.raw=0x80000000, .enable=0x1, },
};

static ac_bool test_pci_cfg_addr(struct test_case* test) {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(test->val.fields.reg == test->reg);
  error |= AC_TEST(test->val.fields.func == test->func);
  error |= AC_TEST(test->val.fields.dev == test->dev);
  error |= AC_TEST(test->val.fields.bus == test->bus);
  error |= AC_TEST(test->val.fields.resv == test->resv);
  error |= AC_TEST(test->val.fields.enable == test->enable);

  if (error) {
    ac_pci_cfg_addr_print("fields: ", test->val.fields, "\n");
  }

  return error;
}

ac_bool test_pci_cfg_addr_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_pci_cfg_addr(&test_case_array[i]);
  }

  return error;
}

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
struct test_case_ac_pci_cfg_hdr0 {
  ac_pci_cfg_hdr val;
  ac_u16 vendor_id;
  ac_u16 device_id;
  ac_u16 command;
  ac_u16 status;
  ac_u8 revision_id;
  ac_u8 prog_if;
  ac_u8 sub_class;
  ac_u8 base_class;
  ac_u8 cache_line_size;
  ac_u8 latency_timer;
  ac_u8 header_type;
  ac_u8 bist;
  ac_u32 base_addrs[6];
  ac_u32 cardbus_cis_ptr;
  ac_u16 subsystem_vendor_id;
  ac_u16 subsystem_id;
  ac_u32 rom_base_addr;
  ac_u32 capabilities:8;
  ac_u32 resv0:24;
  ac_u32 resv1;
  ac_u8 interrupt_line;
  ac_u8 interrupt_pin;
  ac_u8 min_grant;
  ac_u8 max_grant;
};

static struct test_case_ac_pci_cfg_hdr0 test_case_ac_pci_cfg_hdr0_array[] = {
  { .val.raw_u32s[0]=0x00000001, .vendor_id=0x0001, },
  { .val.raw_u32s[0]=0x00008000, .vendor_id=0x8000, },
  { .val.raw_u32s[0]=0x00010000, .device_id=0x0001, },
  { .val.raw_u32s[0]=0x80000000, .device_id=0x8000, },
  { .val.raw_u32s[1]=0x00000001, .command=0x0001, },
  { .val.raw_u32s[1]=0x00008000, .command=0x8000, },
  { .val.raw_u32s[1]=0x00010000, .status=0x0001, },
  { .val.raw_u32s[1]=0x80000000, .status=0x8000, },
  { .val.raw_u32s[2]=0x00000001, .revision_id=0x01, },
  { .val.raw_u32s[2]=0x00000080, .revision_id=0x80, },
  { .val.raw_u32s[2]=0x00000100, .prog_if=0x01, },
  { .val.raw_u32s[2]=0x00008000, .prog_if=0x80, },
  { .val.raw_u32s[2]=0x00010000, .sub_class=0x01, },
  { .val.raw_u32s[2]=0x00800000, .sub_class=0x80, },
  { .val.raw_u32s[2]=0x01000000, .base_class=0x01, },
  { .val.raw_u32s[2]=0x80000000, .base_class=0x80, },
  { .val.raw_u32s[3]=0x00000001, .cache_line_size=0x01, },
  { .val.raw_u32s[3]=0x00000080, .cache_line_size=0x80, },
  { .val.raw_u32s[3]=0x00000100, .latency_timer=0x01, },
  { .val.raw_u32s[3]=0x00008000, .latency_timer=0x80, },
  { .val.raw_u32s[3]=0x00010000, .header_type=0x01, },
  { .val.raw_u32s[3]=0x00800000, .header_type=0x80, },
  { .val.raw_u32s[3]=0x01000000, .bist=0x01, },
  { .val.raw_u32s[3]=0x80000000, .bist=0x80, },
  { .val.raw_u32s[4]=0x00000001, .base_addrs[0]=0x00000001, },
  { .val.raw_u32s[4]=0x80000000, .base_addrs[0]=0x80000000, },
  { .val.raw_u32s[5]=0x00000001, .base_addrs[1]=0x00000001, },
  { .val.raw_u32s[5]=0x80000000, .base_addrs[1]=0x80000000, },
  { .val.raw_u32s[6]=0x00000001, .base_addrs[2]=0x00000001, },
  { .val.raw_u32s[6]=0x80000000, .base_addrs[2]=0x80000000, },
  { .val.raw_u32s[7]=0x00000001, .base_addrs[3]=0x00000001, },
  { .val.raw_u32s[7]=0x80000000, .base_addrs[3]=0x80000000, },
  { .val.raw_u32s[8]=0x00000001, .base_addrs[4]=0x00000001, },
  { .val.raw_u32s[8]=0x80000000, .base_addrs[4]=0x80000000, },
  { .val.raw_u32s[9]=0x00000001, .base_addrs[5]=0x00000001, },
  { .val.raw_u32s[9]=0x80000000, .base_addrs[5]=0x80000000, },
  { .val.raw_u32s[10]=0x00000001, .cardbus_cis_ptr=0x00000001, },
  { .val.raw_u32s[10]=0x80000000, .cardbus_cis_ptr=0x80000000, },
  { .val.raw_u32s[11]=0x00000001, .subsystem_vendor_id=0x0001, },
  { .val.raw_u32s[11]=0x00008000, .subsystem_vendor_id=0x8000, },
  { .val.raw_u32s[11]=0x00010000, .subsystem_id=0x0001, },
  { .val.raw_u32s[11]=0x80000000, .subsystem_id=0x8000, },
  { .val.raw_u32s[12]=0x00000001, .rom_base_addr=0x00000001, },
  { .val.raw_u32s[12]=0x80000000, .rom_base_addr=0x80000000, },
  { .val.raw_u32s[13]=0x00000001, .capabilities=0x01, },
  { .val.raw_u32s[13]=0x00000080, .capabilities=0x80, },
  { .val.raw_u32s[13]=0x00000100, .resv0=0x000001, },
  { .val.raw_u32s[13]=0x80000000, .resv0=0x800000, },
  { .val.raw_u32s[14]=0x00000001, .resv1=0x00000001, },
  { .val.raw_u32s[14]=0x80000000, .resv1=0x80000000, },
  { .val.raw_u32s[15]=0x00000001, .interrupt_line=0x01, },
  { .val.raw_u32s[15]=0x00000080, .interrupt_line=0x80, },
  { .val.raw_u32s[15]=0x00000100, .interrupt_pin=0x01, },
  { .val.raw_u32s[15]=0x00008000, .interrupt_pin=0x80, },
  { .val.raw_u32s[15]=0x00010000, .min_grant=0x01, },
  { .val.raw_u32s[15]=0x00800000, .min_grant=0x80, },
  { .val.raw_u32s[15]=0x01000000, .max_grant=0x01, },
  { .val.raw_u32s[15]=0x80000000, .max_grant=0x80, },
};

static ac_bool test_pci_cfg_hdr0(struct test_case_ac_pci_cfg_hdr0* test) {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(test->val.hdr0.common_hdr.vendor_id == test->vendor_id);
  error |= AC_TEST(test->val.hdr0.common_hdr.device_id == test->device_id);
  error |= AC_TEST(test->val.hdr0.common_hdr.command == test->command);
  error |= AC_TEST(test->val.hdr0.common_hdr.status == test->status);
  error |= AC_TEST(test->val.hdr0.common_hdr.revision_id == test->revision_id);
  error |= AC_TEST(test->val.hdr0.common_hdr.prog_if == test->prog_if);
  error |= AC_TEST(test->val.hdr0.common_hdr.sub_class == test->sub_class);
  error |= AC_TEST(test->val.hdr0.common_hdr.base_class == test->base_class);
  error |= AC_TEST(test->val.hdr0.common_hdr.cache_line_size == test->cache_line_size);
  error |= AC_TEST(test->val.hdr0.common_hdr.latency_timer == test->latency_timer);
  error |= AC_TEST(test->val.hdr0.common_hdr.header_type == test->header_type);
  error |= AC_TEST(test->val.hdr0.common_hdr.bist == test->bist);
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test->base_addrs); i++) {
    error |= AC_TEST(test->val.hdr0.base_addrs[i] == test->base_addrs[i]);
  }
  error |= AC_TEST(test->val.hdr0.cardbus_cis_ptr == test->cardbus_cis_ptr);
  error |= AC_TEST(test->val.hdr0.subsystem_vendor_id == test->subsystem_vendor_id);
  error |= AC_TEST(test->val.hdr0.subsystem_id == test->subsystem_id);
  error |= AC_TEST(test->val.hdr0.rom_base_addr == test->rom_base_addr);
  error |= AC_TEST(test->val.hdr0.capabilities == test->capabilities);
  error |= AC_TEST(test->val.hdr0.resv0 == test->resv0);
  error |= AC_TEST(test->val.hdr0.resv1 == test->resv1);
  error |= AC_TEST(test->val.hdr0.interrupt_line == test->interrupt_line);
  error |= AC_TEST(test->val.hdr0.interrupt_pin == test->interrupt_pin);
  error |= AC_TEST(test->val.hdr0.min_grant == test->min_grant);
  error |= AC_TEST(test->val.hdr0.max_grant == test->max_grant);

  if (error) {
    ac_printf("hdr0 fields:\n");
    ac_pci_cfg_hdr0_print("  ", &test->val.hdr0);
  }

  return error;
}

ac_bool test_pci_cfg_hdr0_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_ac_pci_cfg_hdr0_array); i++) {
    error |= test_pci_cfg_hdr0(&test_case_ac_pci_cfg_hdr0_array[i]);
  }

  return error;
}


/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
struct test_case_ac_pci_cfg_hdr1 {
  ac_pci_cfg_hdr val;
  ac_u16 vendor_id;
  ac_u16 device_id;
  ac_u16 command;
  ac_u16 status;
  ac_u8 revision_id;
  ac_u8 prog_if;
  ac_u8 sub_class;
  ac_u8 base_class;
  ac_u8 cache_line_size;
  ac_u8 latency_timer;
  ac_u8 header_type;
  ac_u8 bist;
  ac_u32 base_addrs[2];
  ac_u8 primary_bus_number;
  ac_u8 secondary_bus_number;
  ac_u8 subordinate_bus_number;
  ac_u8 secondary_latency_timer;
  ac_u8 io_base;
  ac_u8 io_limit;
  ac_u16 secondary_status;
  ac_u16 memory_base;
  ac_u16 memory_limit;
  ac_u16 prefetchable_memory_base;
  ac_u16 prefetchable_memory_limit;
  ac_u32 prefetchable_hi32_memory_base;
  ac_u32 prefetchable_hi32_memory_limit;
  ac_u16 io_hi16_base;
  ac_u16 io_hi16_limit;
  ac_u32 capabilities:8;
  ac_u32 resv0:24;
  ac_u32 expansion_rom_base;
  ac_u8 interrupt_line;
  ac_u8 interrupt_pin;
  ac_u16 bridge_control;
};

static struct test_case_ac_pci_cfg_hdr1 test_case_ac_pci_cfg_hdr1_array[] = {
  { .val.raw_u32s[0]=0x00000001, .vendor_id=0x0001, },
  { .val.raw_u32s[0]=0x00008000, .vendor_id=0x8000, },
  { .val.raw_u32s[0]=0x00010000, .device_id=0x0001, },
  { .val.raw_u32s[0]=0x80000000, .device_id=0x8000, },
  { .val.raw_u32s[1]=0x00000001, .command=0x0001, },
  { .val.raw_u32s[1]=0x00008000, .command=0x8000, },
  { .val.raw_u32s[1]=0x00010000, .status=0x0001, },
  { .val.raw_u32s[1]=0x80000000, .status=0x8000, },
  { .val.raw_u32s[2]=0x00000001, .revision_id=0x01, },
  { .val.raw_u32s[2]=0x00000080, .revision_id=0x80, },
  { .val.raw_u32s[2]=0x00000100, .prog_if=0x01, },
  { .val.raw_u32s[2]=0x00008000, .prog_if=0x80, },
  { .val.raw_u32s[2]=0x00010000, .sub_class=0x01, },
  { .val.raw_u32s[2]=0x00800000, .sub_class=0x80, },
  { .val.raw_u32s[2]=0x01000000, .base_class=0x01, },
  { .val.raw_u32s[2]=0x80000000, .base_class=0x80, },
  { .val.raw_u32s[3]=0x00000001, .cache_line_size=0x01, },
  { .val.raw_u32s[3]=0x00000080, .cache_line_size=0x80, },
  { .val.raw_u32s[3]=0x00000100, .latency_timer=0x01, },
  { .val.raw_u32s[3]=0x00008000, .latency_timer=0x80, },
  { .val.raw_u32s[3]=0x00010000, .header_type=0x01, },
  { .val.raw_u32s[3]=0x00800000, .header_type=0x80, },
  { .val.raw_u32s[3]=0x01000000, .bist=0x01, },
  { .val.raw_u32s[3]=0x80000000, .bist=0x80, },
  { .val.raw_u32s[4]=0x00000001, .base_addrs[0]=0x00000001, },
  { .val.raw_u32s[4]=0x80000000, .base_addrs[0]=0x80000000, },
  { .val.raw_u32s[5]=0x00000001, .base_addrs[1]=0x00000001, },
  { .val.raw_u32s[5]=0x80000000, .base_addrs[1]=0x80000000, },
  { .val.raw_u32s[6]=0x00000001, .primary_bus_number=0x01, },
  { .val.raw_u32s[6]=0x00000080, .primary_bus_number=0x80, },
  { .val.raw_u32s[6]=0x00000100, .secondary_bus_number=0x01, },
  { .val.raw_u32s[6]=0x00008000, .secondary_bus_number=0x80, },
  { .val.raw_u32s[6]=0x00010000, .subordinate_bus_number=0x01, },
  { .val.raw_u32s[6]=0x00800000, .subordinate_bus_number=0x80, },
  { .val.raw_u32s[6]=0x01000000, .secondary_latency_timer=0x01, },
  { .val.raw_u32s[6]=0x80000000, .secondary_latency_timer=0x80, },
  { .val.raw_u32s[7]=0x00000001, .io_base=0x01, },
  { .val.raw_u32s[7]=0x00000080, .io_base=0x80, },
  { .val.raw_u32s[7]=0x00000100, .io_limit=0x01, },
  { .val.raw_u32s[7]=0x00008000, .io_limit=0x80, },
  { .val.raw_u32s[7]=0x00010000, .secondary_status=0x0001, },
  { .val.raw_u32s[7]=0x80000000, .secondary_status=0x8000, },
  { .val.raw_u32s[8]=0x00000001, .memory_base=0x0001, },
  { .val.raw_u32s[8]=0x00008000, .memory_base=0x8000, },
  { .val.raw_u32s[8]=0x00010000, .memory_limit=0x0001, },
  { .val.raw_u32s[8]=0x80000000, .memory_limit=0x8000, },
  { .val.raw_u32s[9]=0x00000001, .prefetchable_memory_base=0x0001, },
  { .val.raw_u32s[9]=0x00008000, .prefetchable_memory_base=0x8000, },
  { .val.raw_u32s[9]=0x00010000, .prefetchable_memory_limit=0x0001, },
  { .val.raw_u32s[9]=0x80000000, .prefetchable_memory_limit=0x8000, },
  { .val.raw_u32s[10]=0x00000001, .prefetchable_hi32_memory_base=0x00000001, },
  { .val.raw_u32s[10]=0x80000000, .prefetchable_hi32_memory_base=0x80000000, },
  { .val.raw_u32s[11]=0x00000001, .prefetchable_hi32_memory_limit=0x00000001, },
  { .val.raw_u32s[11]=0x80000000, .prefetchable_hi32_memory_limit=0x80000000, },
  { .val.raw_u32s[12]=0x00000001, .io_hi16_base=0x0001, },
  { .val.raw_u32s[12]=0x00008000, .io_hi16_base=0x8000, },
  { .val.raw_u32s[12]=0x00010000, .io_hi16_limit=0x0001, },
  { .val.raw_u32s[12]=0x80000000, .io_hi16_limit=0x8000, },
  { .val.raw_u32s[13]=0x00000001, .capabilities=0x01, },
  { .val.raw_u32s[13]=0x00000080, .capabilities=0x80, },
  { .val.raw_u32s[13]=0x00000100, .resv0=0x000001, },
  { .val.raw_u32s[13]=0x80000000, .resv0=0x800000, },
  { .val.raw_u32s[14]=0x00000001, .expansion_rom_base=0x00000001, },
  { .val.raw_u32s[14]=0x80000000, .expansion_rom_base=0x80000000, },
  { .val.raw_u32s[15]=0x00000001, .interrupt_line=0x01, },
  { .val.raw_u32s[15]=0x00000080, .interrupt_line=0x80, },
  { .val.raw_u32s[15]=0x00000100, .interrupt_pin=0x01, },
  { .val.raw_u32s[15]=0x00008000, .interrupt_pin=0x80, },
  { .val.raw_u32s[15]=0x00010000, .bridge_control=0x0001, },
  { .val.raw_u32s[15]=0x80000000, .bridge_control=0x8000, },
};

static ac_bool test_pci_cfg_hdr1(struct test_case_ac_pci_cfg_hdr1* test) {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(test->val.hdr0.common_hdr.vendor_id == test->vendor_id);
  error |= AC_TEST(test->val.hdr0.common_hdr.device_id == test->device_id);
  error |= AC_TEST(test->val.hdr0.common_hdr.command == test->command);
  error |= AC_TEST(test->val.hdr0.common_hdr.status == test->status);
  error |= AC_TEST(test->val.hdr0.common_hdr.revision_id == test->revision_id);
  error |= AC_TEST(test->val.hdr0.common_hdr.prog_if == test->prog_if);
  error |= AC_TEST(test->val.hdr0.common_hdr.sub_class == test->sub_class);
  error |= AC_TEST(test->val.hdr0.common_hdr.base_class == test->base_class);
  error |= AC_TEST(test->val.hdr0.common_hdr.cache_line_size == test->cache_line_size);
  error |= AC_TEST(test->val.hdr0.common_hdr.latency_timer == test->latency_timer);
  error |= AC_TEST(test->val.hdr0.common_hdr.header_type == test->header_type);
  error |= AC_TEST(test->val.hdr0.common_hdr.bist == test->bist);
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test->base_addrs); i++) {
    error |= AC_TEST(test->val.hdr1.base_addrs[i] == test->base_addrs[i]);
  }
  error |= AC_TEST(test->val.hdr1.primary_bus_number == test->primary_bus_number);
  error |= AC_TEST(test->val.hdr1.secondary_bus_number == test->secondary_bus_number);
  error |= AC_TEST(test->val.hdr1.subordinate_bus_number == test->subordinate_bus_number);
  error |= AC_TEST(test->val.hdr1.secondary_latency_timer == test->secondary_latency_timer);
  error |= AC_TEST(test->val.hdr1.io_base == test->io_base);
  error |= AC_TEST(test->val.hdr1.io_limit == test->io_limit);
  error |= AC_TEST(test->val.hdr1.secondary_status == test->secondary_status);
  error |= AC_TEST(test->val.hdr1.memory_base == test->memory_base);
  error |= AC_TEST(test->val.hdr1.memory_limit == test->memory_limit);
  error |= AC_TEST(test->val.hdr1.prefetchable_memory_base == test->prefetchable_memory_base);
  error |= AC_TEST(test->val.hdr1.prefetchable_memory_limit == test->prefetchable_memory_limit);
  error |= AC_TEST(test->val.hdr1.prefetchable_hi32_memory_base == test->prefetchable_hi32_memory_base);
  error |= AC_TEST(test->val.hdr1.prefetchable_hi32_memory_limit == test->prefetchable_hi32_memory_limit);
  error |= AC_TEST(test->val.hdr1.io_hi16_base == test->io_hi16_base);
  error |= AC_TEST(test->val.hdr1.io_hi16_limit == test->io_hi16_limit);
  error |= AC_TEST(test->val.hdr1.capabilities == test->capabilities);
  error |= AC_TEST(test->val.hdr1.resv0 == test->resv0);
  error |= AC_TEST(test->val.hdr1.expansion_rom_base == test->expansion_rom_base);
  error |= AC_TEST(test->val.hdr1.interrupt_line == test->interrupt_line);
  error |= AC_TEST(test->val.hdr1.interrupt_pin == test->interrupt_pin);
  error |= AC_TEST(test->val.hdr1.bridge_control == test->bridge_control);

  if (error) {
    ac_printf("hdr1 fields:\n");
    ac_pci_cfg_hdr1_print("  ", &test->val.hdr1);
  }

  return error;
}

ac_bool test_pci_cfg_hdr1_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_ac_pci_cfg_hdr1_array); i++) {
    error |= test_pci_cfg_hdr1(&test_case_ac_pci_cfg_hdr1_array[i]);
  }

  return error;
}

/**
 * Forward declartion
 */
void visit_all_devices(ac_pci_cfg_addr cfg_addr);

/**
 * Visit a device and recursively call visit_all_devices
 * if the devices base_class is a bridge (0x6) and it has
 * a header_type of 1 which indicates its a type of pci bridge.
 */
void visit_device(ac_pci_cfg_addr cfg_addr) {
  ac_u16 vendor_id = ac_pci_cfg_get_vendor_id(cfg_addr);
  if (vendor_id != 0xFFFF) {
    ac_pci_cfg_hdr header;
    ac_pci_cfg_hdr_get(cfg_addr, &header);
    ac_pci_cfg_addr_print("cfg_addr: ", cfg_addr, "\n");
    ac_pci_cfg_hdr_print("  ", &header);
    if ((header.hdr0.common_hdr.base_class == 0x6) &&
        ((header.hdr0.common_hdr.header_type & 0x7f) == 1)) {
      ac_pci_cfg_addr sub_bus = cfg_addr;
      sub_bus.bus = header.hdr1.secondary_bus_number;
      visit_all_devices(sub_bus);
    }
  }
}

/**
 * Visit all devices on the bus
 */
void visit_all_devices(ac_pci_cfg_addr cfg_addr) {
  cfg_addr.func = 0;
  for (ac_uint dev = 0; dev <= MAX_DEV; dev++) {
    cfg_addr.dev = dev;
    ac_u16 vendor_id = ac_pci_cfg_get_vendor_id(cfg_addr);
    if (vendor_id != 0xFFFF) {
      if ((ac_pci_cfg_get_header_type(cfg_addr) & 0x80) == 0x80) {
        for (ac_uint func = 0; func <= MAX_FUNC; func++) {
          ac_pci_cfg_addr func_addr = cfg_addr;
          func_addr.func = func;
          visit_device(func_addr);
        }
      } else {
        visit_device(cfg_addr);
      }
    }
  }
}

/**
 * This visit all buses.
 * [See](http://wiki.osdev.org/PCI).
 */
void visit_all_buses(void) {
  ac_pci_cfg_addr cfg_addr = ac_pci_cfg_addr_init(0, 0, 0, 0);
  if ((ac_pci_cfg_get_header_type(cfg_addr) & 0x80) == 0) {
    visit_all_devices(cfg_addr);
  } else {
    for (ac_uint func = 0; func <= MAX_FUNC; func++) {
      ac_pci_cfg_addr cfg_addr = ac_pci_cfg_addr_init(0, 0, func, 0);
      visit_all_devices(cfg_addr);
    }
  }
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_pci_cfg_addr_fields();
  error |= test_pci_cfg_hdr0_fields();
  error |= test_pci_cfg_hdr1_fields();

  visit_all_buses();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
