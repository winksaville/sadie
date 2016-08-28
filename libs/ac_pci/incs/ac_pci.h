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

#ifndef SADIE_LIBS_AC_PCI_INCS_AC_PCI_H
#define SADIE_LIBS_AC_PCI_INCS_AC_PCI_H

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_bits.h>
#include <ac_inttypes.h>

#define MAX_FUNC 0x7
#define MAX_DEV  0x1F
#define MAX_BUS  0xFF

struct AC_ATTR_PACKED ac_pci_cfg_addr {
  union {
    ac_u32 raw;                 // u32 version of the cfg_addr
    struct {
      ac_u32 reg:8;             // Register
      ac_u32 func:3;            // Function on the device (sub-dev)
      ac_u32 dev:5;             // Device
      ac_u32 bus:8;             // Bus
      ac_u32 resv:7;            // Reserved
      ac_bool enable:1;         // Enable bit
    };
  };
};

typedef struct ac_pci_cfg_addr  ac_pci_cfg_addr;

ac_static_assert(sizeof(ac_pci_cfg_addr) == 4,
    L"ac_pci_cfg_addr is not 4 bytes");

struct AC_ATTR_PACKED ac_pci_cfg_hdr_cmn {
  union {
    ac_u32 raw[4];
    struct {
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
      ac_u8 hdr_type:7;
      ac_bool multi_func:1;
      ac_u8 bist;
    };
  };
};

typedef struct ac_pci_cfg_hdr_cmn ac_pci_cfg_hdr_cmn;

ac_static_assert(sizeof(ac_pci_cfg_hdr_cmn) == 16,
    L"ac_pci_cfg_hdr_cmn is not 16 bytes");


/**
 * pci type 0 configuration header
 */
struct AC_ATTR_PACKED ac_pci_cfg_hdr0 {
  union {
    ac_u32 raw[12];
    struct {
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
  };
};

typedef struct ac_pci_cfg_hdr0 ac_pci_cfg_hdr0;

ac_static_assert(sizeof(ac_pci_cfg_hdr0) == 48,
    L"ac_pci_cfg_hdr0 is not 48 bytes");

/**
 * pci configuration header type1
 */
struct AC_ATTR_PACKED ac_pci_cfg_hdr1 {
  union {
    ac_u32 raw[12];
    struct {
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
  };
};

typedef struct ac_pci_cfg_hdr1 ac_pci_cfg_hdr1;

ac_static_assert(sizeof(ac_pci_cfg_hdr1) == 48,
    L"ac_pci_cfg_hdr1 is not 48 bytes");

struct AC_ATTR_PACKED ac_pci_cfg_hdr {
  ac_pci_cfg_hdr_cmn hdr_cmn;
  union {
    struct ac_pci_cfg_hdr0 hdr0;
    struct ac_pci_cfg_hdr1 hdr1;
  };
};

typedef struct ac_pci_cfg_hdr ac_pci_cfg_hdr;

ac_static_assert(sizeof(ac_pci_cfg_hdr) == 64,
    L"ac_pci_cfg_hdr is not 64 bytes");

/**
 * pci configuration header type2 is for cardbus
 * which is no longer used on modern systems so there
 * is no easy way to test, for now I'll not implement.
 */

/**
 * Initialize ac_pci_cfg_addr
 */
static inline ac_pci_cfg_addr ac_pci_cfg_addr_init(ac_u32 bus, ac_u32 dev,
    ac_u32 func, ac_u32 reg) {
  ac_pci_cfg_addr cfg_addr = {
    .enable=1, .resv=0, .bus=bus, .dev=dev, .func=func, .reg=reg };
  return  cfg_addr;
}

/*
 * Include the implemenation of ac_pci_cfg_rd_u32 and wr_u32.
 */
#include <ac_pci_impl.h>

/*
 * pci configuration read u32.
 *
 * @return 0xFFFFFFFF if that address doesn't exist.
 */
//ac_u32 ac_pci_cfg_rd_u32(pci_cfg_addr cfg_addr);

/*
 * pci configuration write u32
 */
//ac_u32 pci_cfg_wr_u32(pci_cfg_addr cfg_addr, ac_u32 val);


/**
 * pci configuration read u16
 */
static inline ac_u16 ac_pci_cfg_rd_u16(ac_pci_cfg_addr cfg_addr) {
  ac_u32 val_u32 = ac_pci_cfg_rd_u32(cfg_addr);
  ac_u16 val_u16 = AC_GET_BITS(ac_u16, val_u32, 8 * (cfg_addr.reg & 0x2), 16);
  return val_u16;
}

/**
 * pci configuration read u8
 */
static inline ac_u8 ac_pci_cfg_rd_u8(ac_pci_cfg_addr cfg_addr) {
  ac_u32 val_u32 = ac_pci_cfg_rd_u32(cfg_addr);
  ac_u8 val_u8 = AC_GET_BITS(ac_u8, val_u32, 8 * (cfg_addr.reg & 0x3), 8);
  return val_u8;
}

/**
 * pci configuration write u16
 */
static inline void ac_pci_cfg_wr_u16(ac_pci_cfg_addr cfg_addr, ac_u16 val) {
  ac_u32 val_u32 = ac_pci_cfg_rd_u32(cfg_addr);
  val_u32 = AC_SET_BITS(ac_u32, val_u32, val, 16 * (cfg_addr.reg & 0x1), 16);
  ac_pci_cfg_wr_u32(cfg_addr, val_u32);
}

/**
 * pci configuration write u8
 */
static inline void ac_pci_cfg_wr_u8(ac_pci_cfg_addr cfg_addr, ac_u8 val) {
  ac_u32 val_u32 = ac_pci_cfg_rd_u32(cfg_addr);
  val_u32 = AC_SET_BITS(ac_u32, val_u32, val, 8 * (cfg_addr.reg & 0x3), 8);
  ac_pci_cfg_wr_u32(cfg_addr, val_u32);
}

/**
 * Get PCI vendor Id
 *
 * @return 0xFFFF if no vendor id
 */
ac_u16 ac_pci_cfg_get_vendor_id(ac_pci_cfg_addr addr);

/**
 * Get PCI header type
 *
 * @return 0xFF if no header type
 */
ac_u8 ac_pci_cfg_get_hdr_type(ac_pci_cfg_addr addr);

/**
 * Get PCI multi_func
 */
ac_bool ac_pci_cfg_get_multi_func(ac_pci_cfg_addr addr);

/**
 * Get PCI hdr for the addr
 *
 * @return 0 if successfull
 */
ac_uint ac_pci_cfg_hdr_get(ac_pci_cfg_addr addr, ac_pci_cfg_hdr* header);

#endif
