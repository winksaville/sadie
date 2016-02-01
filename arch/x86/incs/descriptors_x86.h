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

#ifndef ARCH_X86_INCS_DESCRIPTORS_X86_64_h
#define ARCH_X86_INCS_DESCRIPTORS_X86_64_h

#include <ac_inttypes.h>
#include <ac_architecture.h>

#if !defined(ARCH_X86) && (!defined(CPU_X86_32) || !defined(CPU_X86_64))
#error  "Expecting ARCH_X86 && (CPU_X86_32 || CPU_X86_64)"
#endif

#ifdef CPU_X86_64 
#define IDT_INTR_GATE_OFFSET_HI_MASK  ((ac_uptr)0xFFFFFFFFFFFF)
#define IDT_INTR_GATE_SIZE  16
#define TSS_DESC_BASE_ADDR_HI_MASK  ((ac_uptr)0xFFFFFFFFFFFF)
#define TSS_DESC_SIZE  16
#define SEG_DESC_SIZE  16
#else /* CPU_X86_32 */
#define IDT_INTR_GATE_OFFSET_HI_MASK  ((ac_uptr)0xFFFF)
#define IDT_INTR_GATE_SIZE  8;
#define TSS_DESC_BASE_ADDR_HI_MASK  ((ac_uptr)0xFFFF)
#define TSS_DESC_SIZE  8
#define SEG_DESC_SIZE  8
#endif

#define IDT_TASK_GATE_SIZE  8

/**
 * Interrupt Gate, this is the same as a trap gate with a different type.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 6.11 "IDT Descriptors"
 * Figure 6-2. IDT Gate Descriptors
 *
 * and
 *
 * Volume 3 chapter 6.14.1 "64-Bit Mode IDT"
 * Figure 6-7. 64-Bit IDT Gate Descriptors
 */
struct idt_intr_gate {
  ac_uptr offset_lo:16;
  ac_uptr segment:16;
#ifdef CPU_X86_64
  ac_uptr ist:3;
  ac_uptr unused_1:5;
#else /* CPU_X86_32 */
  ac_uptr unused_1:8;
#endif
  ac_uptr type:4;
  ac_uptr unused_2:1;
  ac_uptr dpl:2;
  ac_uptr p:1;
#ifdef CPU_X86_64
  ac_uptr offset_hi:48;
  ac_uptr unused_3:32;
#else /* CPU_X86_32 */
  ac_uptr offset_hi:16;
#endif
} __attribute__((__packed__));

#define xstr(s) str(s)
#define str(s) #s

_Static_assert(sizeof(struct idt_intr_gate) == IDT_INTR_GATE_SIZE,
    L"struct idt_intr_gate is not " xstr(IDT_INTR_GATE_SIZE_STR) " bytes");

typedef struct idt_intr_gate idt_intr_gate;

struct idt_task_gate {
  ac_uptr unused_1:16;
  ac_uptr tss_seg_selector:16;
  ac_uptr unused_2:8;
  ac_uptr type:4;
  ac_uptr unused_3:1;
  ac_uptr dpl:2;
  ac_uptr p:1;
  ac_uptr unused_4:16;
} __attribute__((__packed__));

_Static_assert(sizeof(struct idt_task_gate) == IDT_TASK_GATE_SIZE,
    L"struct intr_trap_gate is not 16 bytes");

typedef struct idt_task_gate idt_task_gate;


#define IDT_INTR_GATE_COMMON_INITIALIZER \
   .offset_lo = 0, \
   .segment = 0, \
   .unused_1 = 0, \
   .type = 0, \
   .unused_2 = 0, \
   .dpl = 0, \
   .p = 0, \
   .offset_hi = 0, \

#define IDT_INTR_GATE_X86_64_EXTRA_INITIALIZER \
  .ist = 0, \
  .unused_3 = 0,

#ifdef CPU_X86_64
#define IDT_INTR_GATE_INITIALIZER { \
   IDT_INTR_GATE_COMMON_INITIALIZER \
   IDT_INTR_GATE_X86_64_EXTRA_INITIALIZER \
}
#else /* CPU_X86_32 */
#define IDT_INTR_GATE_INITIALIZER { \
   IDT_COMMON_INITIALIZER \
}
#endif

/** Return the bits for idt_intr_gate.offset_lo as a ac_uptr */
#define IDT_INTR_GATE_OFFSET_LO(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 0) & 0xFFFF; \
  r; \
})

/** Return the bits for idt_intr_gate.offset_hi as a ac_uptr */
#define IDT_INTR_GATE_OFFSET_HI(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 16) & IDT_INTR_GATE_OFFSET_HI_MASK \
  r; \
})

/** Return the idt_intr_gate.offset as an ac_uptr */
#define IDT_INTR_GET_GATE_OFFSET(gate) ({ \
  ac_uptr r = (ac_uptr)((((uptr)(gate).offset_hi) << 16) \
      | (ac_uptr)((gate).offset_lo)); \
  r; \
})


/**
 * TSS Descriptor, this is the same as an LDT Descriptor with a different type.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 7.2.3 "TSS Descriptor in 64-bit mode"
 * Figure 7-4. Format of TSS and LDT Descriptors in 64-bit Mode
 *
 * and
 *
 * Volume 3 chapter 7.2.2 "TSS Descriptor"
 * Figure 7-3. Format of TSS Descriptor
 */
struct tss_desc {
  ac_uptr seg_limit_lo:16;
  ac_uptr base_addr_lo:24;
  ac_uptr type:4;
  ac_uptr unused_1:1;
  ac_uptr dpl:2;
  ac_uptr p:1;
  ac_uptr seg_limit_hi:4;
  ac_uptr avl:1;
  ac_uptr unused_2:2;
  ac_uptr g:1;
#ifdef CPU_X86_64
  ac_uptr base_addr_hi:40;
  ac_uptr unused_3:32;
#else /* CPU_X86_32 */
  ac_uptr base_addr_hi:8;
#endif
} __attribute__((__packed__));

_Static_assert(sizeof(struct tss_desc) == TSS_DESC_SIZE,
    L"TSS/LDT segment_descriptor is not " xstr(TSS_DESC_SIZE) " bytes");

typedef struct tss_ldt_desc tss_desc;

#define TSS_DESC_COMMON_INITIALIZER { \
  .seg_limit_lo = 0, \
  .base_addr_lo = 0, \
  .type = 0, \
  .unused_1 = 0, \
  .dpl = 0, \
  .p = 0, \
  .seg_limit_hi = 0, \
  .avl = 0, \
  .unused_2 = 0, \
  .g = 0, \
  .base_addr_hi = 0, \
}

#ifdef CPU_X86_64
#define TSS_DESC_INITIALIZER { \
  TSS_DESC_COMMON_INITIALIZER \
  unused_3 = 0, \
}
#else /* CPU_X86_32 */
#define TSS_DESC_INITIALIZER { \
  TSS_DESC_COMMON_INITIALIZER \
}
#endif

/** Return the bits for tss_ldt_desc.seg_limit_lo as an ac_uptr */
#define TSS_DESC_SEG_LIMIT_LO(sl) ({ \
  ac_uptr r = (((ac_uptr)(sl) >> 0) & 0xFFFF); \
  r; \
})

/** Return the bits for tss_ldt_desc.seg_limit_hi as an ac_uptr */
#define TSS_DESC_SEG_LIMIT_HI(sl) ({ \
  ac_uptr r = ((ac_uptr)(sl) >> 16) & 0xF; \
  r; \
})

/** Return tss_ldt_desc.seg_limit as an ac_uptr */
#define GET_TSS_DESC_SEG_LIMIT(desc) ({ \
  ac_uptr r = (ac_uptr)((((ac_uptr)(desc).seg_limit_hi) << 16) \
      | (ac_uptr)((desc).seg_limit_lo)); \
  r; \
})

/** Return the bits for tss_ldt_desc.base_addr_lo */
#define TSS_DESC_BASE_ADDR_LO(addr) ({ \
  ac_uptr r = (((ac_uptr)(addr) >> 16) & ((ac_uptr)0xFFFFFF)); \
  r; \
})

/** Return the bits for tss_ldt_desc.base_addr_hi */
#define TSS_DESC_BASE_ADDR_HI(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 16) & TSS_DESC_BASE_ADDR_HI_MASK; \
  r; \
})

/** Return tss_desc.base_addr as a ac_uptr */
#define GET_TSS_LDT_DESC_BASE_ADDR(desc) ({ \
  ac_uptr r = (ac_uptr)((((ac_uptr)(desc).base_addr_hi) << 24) \
      | (ac_uptr)((desc).base_addr_lo)); \
  r; \
})

/**
 * Segment Descriptor
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 3.4.5 "Segment Descriptors"
 * Figure 3-8. Segment Descriptor
 */
struct seg_desc {
  ac_u32 seg_limit_lo:16;
  ac_u32 base_addr_lo:24;
  ac_u32 type:4;
  ac_u32 s:1;
  ac_u32 dpl:2;
  ac_u32 p:1;
  ac_u32 seg_limit_hi:4;
  ac_u32 avl:1;
  ac_u32 l:1;
  ac_u32 d_b:1;
  ac_u32 g:1;
  ac_u32 base_addr_hi:8;
} __attribute__((__packed__));

_Static_assert(sizeof(struct seg_desc) == 8,
    L"segment_descriptor is not 8 bytes");

typedef struct seg_desc seg_desc;

#define SEG_DESC_INITIALIZER { \
  .seg_limit_lo = 0, \
  .base_addr_lo = 0, \
  .type = 0, \
  .s = 0, \
  .dpl = 0, \
  .p = 0, \
  .seg_limit_hi = 0, \
  .avl = 0, \
  .l = 0, \
  .d_b = 0, \
  .g = 0, \
  .base_addr_hi = 0, \
}

/** Return the bits for seg_desc.seg_limit_lo */
#define SEG_DESC_SEG_LIMIT_LO(i) ({ \
  ac_uptr r = (((ac_uptr)(i) >> 0) & 0xFFFF); \
  r; \
})

/** Return the bits for seg_desc.seg_limit_hi */
#define SEG_DESC_SEG_LIMIT_HI(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 16) & 0xFFFFFFFFFFFFLL; \
  r; \
})

/** Return seg_desc.seg_limit as a ac_uptr */
#define GET_SEG_DESC_SEG_LIMIT(sd) ({ \
  ac_uptr r = (ac_uptr)((((uptr)(sd).seg_limit_hi) << 16) \
      | (ac_uptr)((sd).seg_limit_lo)); \
  r; \
})

/** Return the bits for seg_desc.base_addr_lo */
#define SEG_DESC_BASE_ADDR_LO(addr) ({ \
  ac_uptr r = (((ac_uptr)(addr) >> 16) & 0xFFFFFF); \
  r; \
})

/** Return the bits for seg_desc.base_addr_hi */
#define SEG_DESC_BASE_ADDR_HI(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 24) & 0xFF; \
  r; \
})

/** Return seg_desc.base_addr as a ac_uptr */
#define GET_SEG_DESC_BASE_ADDR(desc) ({ \
  ac_uptr r = (ac_uptr)((((uptr)(desc).base_addr_hi) << 24) \
      | (ac_uptr)((desc).base_addr_lo)); \
  r; \
})

/**
 * System and Gate Desctriptor Types for 64 bit IA32e
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 3.5 "Segment Descriptor Types"
 * Table 3-2. System-Segment and Gate-Descriptor Types
 */
enum seg_type_sys_gate {
  DT_64_UPPER =      0,
  DT_64_RESV_1 =     1,
  DT_64_LDT =        2,
  DT_64_RESV_3 =     3,
  DT_64_RESV_4 =     4,
  DT_64_RESV_5 =     5,
  DT_64_RESV_6 =     6,
  DT_64_RESV_7 =     7,
  DT_64_RESV_8 =     8,
  DT_64_TSS_AVAIL =  9,
  DT_64_RESV_10 =   10,
  DT_64_TSS64_BUSY= 11,
  DT_64_CALL_GATE = 12,
  DT_64_RESV_13 =   13,
  DT_64_INTR_GATE = 14,
  DT_64_TRAP_GATE = 15,
};

/**
 * Code- and Data-Segment Descriptor Types
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 3.4.5.1 "Segment Descriptors"
 * Table 3-1. Code- and Data-Segment Types
 */
struct seg_type_code {
  ac_u8 a:1;       // accessed, set by hardware cleared by software
  ac_u8 r:1;       // readable
  ac_u8 c:1;       // conforming
  ac_u8 one:1;     // for code this bit is always one
} __attribute__((__packed__));
_Static_assert(sizeof(struct seg_type_code) == 1,
    L"sizeof seg_type_code must be one byte");

struct seg_type_data {
  ac_u8 a:1;       // accessed, set by hardware cleared by software
  ac_u8 w:1;       // write enable, 1 == read/write, 0 == read only
  ac_u8 e:1;       // expansion direction down == 1, up == 0
  ac_u8 zero:1;    // for data this bit is always zero
} __attribute__((__packed__));
_Static_assert(sizeof(struct seg_type_data) == 1,
    L"sizeof seg_type_data must be one byte");

union seg_type_u {
    ac_u8 byte;
    struct seg_type_data data;
    struct seg_type_code code;
} __attribute__((__packed__));
_Static_assert(sizeof(union seg_type_u) == 1, L"sizeof seg_type_u must be one byte");

/**
 * Pointer to Global Descriptor Table and Interrupt Descriptor Table
 */
struct descriptor_ptr {
    ac_u16 unused[3];      // Align descriptor_ptr.limit to an odd ac_u16 boundary
                        // so descriptor_ptr.address is on a ac_uptr boundary.
                        // This is for better performance and for user mode
                        // it avoids an alignment check fault. See the last
                        // paragraph of "Intel 64 and IA-32 Architectures
                        // Software Developer's Manual" Volume 3 chapter 3.5.1
                        // "Segment Descriptor Tables".
    volatile ac_u16 limit;
    volatile union {    // Volatile so compiler stores the address in set_idtr.
      ac_uptr address;
      idt_intr_gate* iig;
      seg_desc* sd;
    };
} __attribute__((__packed__, __aligned__(16)));

_Static_assert(sizeof(struct descriptor_ptr) == SEG_DESC_SIZE,
    L"struct descriptor_ptr != " xstr(SEG_DESC_SIZE) " bytes");

/** Descriptor Pointer typedef */
typedef struct descriptor_ptr descriptor_ptr;

void set_seg_desc(seg_desc* sd, ac_u32 seg_limit, ac_uptr base_addr, ac_u8 type,
    ac_u8 s, ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 l, ac_u8 d_b, ac_u8 g);

void set_code_seg_desc(seg_desc* sd, ac_u8 accessed, ac_u8 readable,
    ac_u8 conforming, ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 l, ac_u8 d, ac_u8 g);

void set_data_seg_desc(seg_desc* sd, ac_u8 accessed, ac_u8 write_enable,
    ac_u8 expand_dir, ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 l, ac_u8 d, ac_u8 g);

ac_s32 cmp_seg_desc(seg_desc* sd1, seg_desc* sd2);

void set_tss_ldt_desc(tss_desc* tld, ac_u32 seg_limit, ac_uptr base_addr,
    ac_u8 type, ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 g);

ac_s32 cmp_tss_ldt_desc(tss_desc* sd1, tss_desc* sd2);

#endif
