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

#include <interrupts_x86.h>
#include <interrupts_x86_print.h>

#include <apic_x86.h>

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_xstr.h>

#include <descriptors_x86.h>
#include <native_x86.h>
#include <reset_x86.h>

static void set_intr_gate(IntrGate* gate, intr_handler* ih);

/* Interrupt Descriptor Table */
static IntrGate idt[256];

static ac_u32 intr_divide_err_counter;
static ac_u32 intr_debug_counter;
static ac_u32 intr_nmi_counter;
static ac_u32 intr_breakpoint_counter;
static ac_u32 intr_overflow_counter;
static ac_u32 intr_bound_range_counter;
static ac_u32 intr_invalid_opcode_counter;
static ac_u32 intr_device_not_available_counter;
static ac_u32 expt_double_fault_counter;
static ac_u32 intr_coprocessor_segment_overrun_counter;
static ac_u32 expt_invalid_tss_counter;
static ac_u32 expt_segment_not_present_counter;
static ac_u32 expt_stack_fault_counter;
static ac_u32 expt_general_protection_counter;
static ac_u32 expt_page_fault_counter;
static ac_u32 intr_x87_floating_point_counter;
static ac_u32 expt_alignment_check_counter;
static ac_u32 intr_machine_check_counter;
static ac_u32 intr_simd_floating_point_counter;
static ac_u32 intr_virtualization_counter;

#define INTR_UNDEFINED(idx) \
ac_uint AC_CAT(intr_undefined_counter, idx) = 0; \
AC_ATTR_INTR_HDLR \
static void AC_CAT(intr_undefined, idx) (IntrFrame *frame) { \
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"); \
  print_full_intr_stack_frame("#UD" AC_STR(idx), frame); \
  AC_CAT(intr_undefined_counter, idx)  += 1; \
  ac_printf(" rsp: %p\n", get_sp()); \
  ac_printf(" intr_undefined_counter" AC_STR(idx) ": %d\n", AC_CAT(intr_undefined_counter, idx)); \
  /* reset_x86(); */ \
  send_apic_eoi(); \
} \

/**
 * Some interrupts must not send EOI, for now we
 * assume those will be overridden from the default
 * intr_undefed0xXX.
 */
INTR_UNDEFINED(0x00)
INTR_UNDEFINED(0x01)
INTR_UNDEFINED(0x02)
INTR_UNDEFINED(0x03)
INTR_UNDEFINED(0x04)
INTR_UNDEFINED(0x05)
INTR_UNDEFINED(0x06)
INTR_UNDEFINED(0x07)
INTR_UNDEFINED(0x08)
INTR_UNDEFINED(0x09)
INTR_UNDEFINED(0x0A)
INTR_UNDEFINED(0x0B)
INTR_UNDEFINED(0x0C)
INTR_UNDEFINED(0x0D)
INTR_UNDEFINED(0x0E)
INTR_UNDEFINED(0x0F)

INTR_UNDEFINED(0x10)
INTR_UNDEFINED(0x11)
INTR_UNDEFINED(0x12)
INTR_UNDEFINED(0x13)
INTR_UNDEFINED(0x14)
INTR_UNDEFINED(0x15)
INTR_UNDEFINED(0x16)
INTR_UNDEFINED(0x17)
INTR_UNDEFINED(0x18)
INTR_UNDEFINED(0x19)
INTR_UNDEFINED(0x1A)
INTR_UNDEFINED(0x1B)
INTR_UNDEFINED(0x1C)
INTR_UNDEFINED(0x1D)
INTR_UNDEFINED(0x1E)
INTR_UNDEFINED(0x1F)

INTR_UNDEFINED(0x20)
INTR_UNDEFINED(0x21)
INTR_UNDEFINED(0x22)
INTR_UNDEFINED(0x23)
INTR_UNDEFINED(0x24)
INTR_UNDEFINED(0x25)
INTR_UNDEFINED(0x26)
INTR_UNDEFINED(0x27)
INTR_UNDEFINED(0x28)
INTR_UNDEFINED(0x29)
INTR_UNDEFINED(0x2A)
INTR_UNDEFINED(0x2B)
INTR_UNDEFINED(0x2C)
INTR_UNDEFINED(0x2D)
INTR_UNDEFINED(0x2E)
INTR_UNDEFINED(0x2F)

INTR_UNDEFINED(0x30)
INTR_UNDEFINED(0x31)
INTR_UNDEFINED(0x32)
INTR_UNDEFINED(0x33)
INTR_UNDEFINED(0x34)
INTR_UNDEFINED(0x35)
INTR_UNDEFINED(0x36)
INTR_UNDEFINED(0x37)
INTR_UNDEFINED(0x38)
INTR_UNDEFINED(0x39)
INTR_UNDEFINED(0x3A)
INTR_UNDEFINED(0x3B)
INTR_UNDEFINED(0x3C)
INTR_UNDEFINED(0x3D)
INTR_UNDEFINED(0x3E)
INTR_UNDEFINED(0x3F)

INTR_UNDEFINED(0x40)
INTR_UNDEFINED(0x41)
INTR_UNDEFINED(0x42)
INTR_UNDEFINED(0x43)
INTR_UNDEFINED(0x44)
INTR_UNDEFINED(0x45)
INTR_UNDEFINED(0x46)
INTR_UNDEFINED(0x47)
INTR_UNDEFINED(0x48)
INTR_UNDEFINED(0x49)
INTR_UNDEFINED(0x4A)
INTR_UNDEFINED(0x4B)
INTR_UNDEFINED(0x4C)
INTR_UNDEFINED(0x4D)
INTR_UNDEFINED(0x4E)
INTR_UNDEFINED(0x4F)

INTR_UNDEFINED(0x50)
INTR_UNDEFINED(0x51)
INTR_UNDEFINED(0x52)
INTR_UNDEFINED(0x53)
INTR_UNDEFINED(0x54)
INTR_UNDEFINED(0x55)
INTR_UNDEFINED(0x56)
INTR_UNDEFINED(0x57)
INTR_UNDEFINED(0x58)
INTR_UNDEFINED(0x59)
INTR_UNDEFINED(0x5A)
INTR_UNDEFINED(0x5B)
INTR_UNDEFINED(0x5C)
INTR_UNDEFINED(0x5D)
INTR_UNDEFINED(0x5E)
INTR_UNDEFINED(0x5F)

INTR_UNDEFINED(0x60)
INTR_UNDEFINED(0x61)
INTR_UNDEFINED(0x62)
INTR_UNDEFINED(0x63)
INTR_UNDEFINED(0x64)
INTR_UNDEFINED(0x65)
INTR_UNDEFINED(0x66)
INTR_UNDEFINED(0x67)
INTR_UNDEFINED(0x68)
INTR_UNDEFINED(0x69)
INTR_UNDEFINED(0x6A)
INTR_UNDEFINED(0x6B)
INTR_UNDEFINED(0x6C)
INTR_UNDEFINED(0x6D)
INTR_UNDEFINED(0x6E)
INTR_UNDEFINED(0x6F)

INTR_UNDEFINED(0x70)
INTR_UNDEFINED(0x71)
INTR_UNDEFINED(0x72)
INTR_UNDEFINED(0x73)
INTR_UNDEFINED(0x74)
INTR_UNDEFINED(0x75)
INTR_UNDEFINED(0x76)
INTR_UNDEFINED(0x77)
INTR_UNDEFINED(0x78)
INTR_UNDEFINED(0x79)
INTR_UNDEFINED(0x7A)
INTR_UNDEFINED(0x7B)
INTR_UNDEFINED(0x7C)
INTR_UNDEFINED(0x7D)
INTR_UNDEFINED(0x7E)
INTR_UNDEFINED(0x7F)

INTR_UNDEFINED(0x80)
INTR_UNDEFINED(0x81)
INTR_UNDEFINED(0x82)
INTR_UNDEFINED(0x83)
INTR_UNDEFINED(0x84)
INTR_UNDEFINED(0x85)
INTR_UNDEFINED(0x86)
INTR_UNDEFINED(0x87)
INTR_UNDEFINED(0x88)
INTR_UNDEFINED(0x89)
INTR_UNDEFINED(0x8A)
INTR_UNDEFINED(0x8B)
INTR_UNDEFINED(0x8C)
INTR_UNDEFINED(0x8D)
INTR_UNDEFINED(0x8E)
INTR_UNDEFINED(0x8F)

INTR_UNDEFINED(0x90)
INTR_UNDEFINED(0x91)
INTR_UNDEFINED(0x92)
INTR_UNDEFINED(0x93)
INTR_UNDEFINED(0x94)
INTR_UNDEFINED(0x95)
INTR_UNDEFINED(0x96)
INTR_UNDEFINED(0x97)
INTR_UNDEFINED(0x98)
INTR_UNDEFINED(0x99)
INTR_UNDEFINED(0x9A)
INTR_UNDEFINED(0x9B)
INTR_UNDEFINED(0x9C)
INTR_UNDEFINED(0x9D)
INTR_UNDEFINED(0x9E)
INTR_UNDEFINED(0x9F)

INTR_UNDEFINED(0xA0)
INTR_UNDEFINED(0xA1)
INTR_UNDEFINED(0xA2)
INTR_UNDEFINED(0xA3)
INTR_UNDEFINED(0xA4)
INTR_UNDEFINED(0xA5)
INTR_UNDEFINED(0xA6)
INTR_UNDEFINED(0xA7)
INTR_UNDEFINED(0xA8)
INTR_UNDEFINED(0xA9)
INTR_UNDEFINED(0xAA)
INTR_UNDEFINED(0xAB)
INTR_UNDEFINED(0xAC)
INTR_UNDEFINED(0xAD)
INTR_UNDEFINED(0xAE)
INTR_UNDEFINED(0xAF)

INTR_UNDEFINED(0xB0)
INTR_UNDEFINED(0xB1)
INTR_UNDEFINED(0xB2)
INTR_UNDEFINED(0xB3)
INTR_UNDEFINED(0xB4)
INTR_UNDEFINED(0xB5)
INTR_UNDEFINED(0xB6)
INTR_UNDEFINED(0xB7)
INTR_UNDEFINED(0xB8)
INTR_UNDEFINED(0xB9)
INTR_UNDEFINED(0xBA)
INTR_UNDEFINED(0xBB)
INTR_UNDEFINED(0xBC)
INTR_UNDEFINED(0xBD)
INTR_UNDEFINED(0xBE)
INTR_UNDEFINED(0xBF)

INTR_UNDEFINED(0xC0)
INTR_UNDEFINED(0xC1)
INTR_UNDEFINED(0xC2)
INTR_UNDEFINED(0xC3)
INTR_UNDEFINED(0xC4)
INTR_UNDEFINED(0xC5)
INTR_UNDEFINED(0xC6)
INTR_UNDEFINED(0xC7)
INTR_UNDEFINED(0xC8)
INTR_UNDEFINED(0xC9)
INTR_UNDEFINED(0xCA)
INTR_UNDEFINED(0xCB)
INTR_UNDEFINED(0xCC)
INTR_UNDEFINED(0xCD)
INTR_UNDEFINED(0xCE)
INTR_UNDEFINED(0xCF)

INTR_UNDEFINED(0xD0)
INTR_UNDEFINED(0xD1)
INTR_UNDEFINED(0xD2)
INTR_UNDEFINED(0xD3)
INTR_UNDEFINED(0xD4)
INTR_UNDEFINED(0xD5)
INTR_UNDEFINED(0xD6)
INTR_UNDEFINED(0xD7)
INTR_UNDEFINED(0xD8)
INTR_UNDEFINED(0xD9)
INTR_UNDEFINED(0xDA)
INTR_UNDEFINED(0xDB)
INTR_UNDEFINED(0xDC)
INTR_UNDEFINED(0xDD)
INTR_UNDEFINED(0xDE)
INTR_UNDEFINED(0xDF)

INTR_UNDEFINED(0xE0)
INTR_UNDEFINED(0xE1)
INTR_UNDEFINED(0xE2)
INTR_UNDEFINED(0xE3)
INTR_UNDEFINED(0xE4)
INTR_UNDEFINED(0xE5)
INTR_UNDEFINED(0xE6)
INTR_UNDEFINED(0xE7)
INTR_UNDEFINED(0xE8)
INTR_UNDEFINED(0xE9)
INTR_UNDEFINED(0xEA)
INTR_UNDEFINED(0xEB)
INTR_UNDEFINED(0xEC)
INTR_UNDEFINED(0xED)
INTR_UNDEFINED(0xEE)
INTR_UNDEFINED(0xEF)

INTR_UNDEFINED(0xF0)
INTR_UNDEFINED(0xF1)
INTR_UNDEFINED(0xF2)
INTR_UNDEFINED(0xF3)
INTR_UNDEFINED(0xF4)
INTR_UNDEFINED(0xF5)
INTR_UNDEFINED(0xF6)
INTR_UNDEFINED(0xF7)
INTR_UNDEFINED(0xF8)
INTR_UNDEFINED(0xF9)
INTR_UNDEFINED(0xFA)
INTR_UNDEFINED(0xFB)
INTR_UNDEFINED(0xFC)
INTR_UNDEFINED(0xFD)
INTR_UNDEFINED(0xFE)
INTR_UNDEFINED(0xFF)

void init_idt_to_undefined(void) {
  set_intr_gate(&idt[0x00], intr_undefined0x00);
  set_intr_gate(&idt[0x01], intr_undefined0x01);
  set_intr_gate(&idt[0x02], intr_undefined0x02);
  set_intr_gate(&idt[0x03], intr_undefined0x03);
  set_intr_gate(&idt[0x04], intr_undefined0x04);
  set_intr_gate(&idt[0x05], intr_undefined0x05);
  set_intr_gate(&idt[0x06], intr_undefined0x06);
  set_intr_gate(&idt[0x07], intr_undefined0x07);
  set_intr_gate(&idt[0x08], intr_undefined0x08);
  set_intr_gate(&idt[0x09], intr_undefined0x09);
  set_intr_gate(&idt[0x0A], intr_undefined0x0A);
  set_intr_gate(&idt[0x0B], intr_undefined0x0B);
  set_intr_gate(&idt[0x0C], intr_undefined0x0C);
  set_intr_gate(&idt[0x0D], intr_undefined0x0D);
  set_intr_gate(&idt[0x0E], intr_undefined0x0E);
  set_intr_gate(&idt[0x0F], intr_undefined0x0F);

  set_intr_gate(&idt[0x10], intr_undefined0x10);
  set_intr_gate(&idt[0x11], intr_undefined0x11);
  set_intr_gate(&idt[0x12], intr_undefined0x12);
  set_intr_gate(&idt[0x13], intr_undefined0x13);
  set_intr_gate(&idt[0x14], intr_undefined0x14);
  set_intr_gate(&idt[0x15], intr_undefined0x15);
  set_intr_gate(&idt[0x16], intr_undefined0x16);
  set_intr_gate(&idt[0x17], intr_undefined0x17);
  set_intr_gate(&idt[0x18], intr_undefined0x18);
  set_intr_gate(&idt[0x19], intr_undefined0x19);
  set_intr_gate(&idt[0x1A], intr_undefined0x1A);
  set_intr_gate(&idt[0x1B], intr_undefined0x1B);
  set_intr_gate(&idt[0x1C], intr_undefined0x1C);
  set_intr_gate(&idt[0x1D], intr_undefined0x1D);
  set_intr_gate(&idt[0x1E], intr_undefined0x1E);
  set_intr_gate(&idt[0x1F], intr_undefined0x1F);

  set_intr_gate(&idt[0x20], intr_undefined0x20);
  set_intr_gate(&idt[0x21], intr_undefined0x21);
  set_intr_gate(&idt[0x22], intr_undefined0x22);
  set_intr_gate(&idt[0x23], intr_undefined0x23);
  set_intr_gate(&idt[0x24], intr_undefined0x24);
  set_intr_gate(&idt[0x25], intr_undefined0x25);
  set_intr_gate(&idt[0x26], intr_undefined0x26);
  set_intr_gate(&idt[0x27], intr_undefined0x27);
  set_intr_gate(&idt[0x28], intr_undefined0x28);
  set_intr_gate(&idt[0x29], intr_undefined0x29);
  set_intr_gate(&idt[0x2A], intr_undefined0x2A);
  set_intr_gate(&idt[0x2B], intr_undefined0x2B);
  set_intr_gate(&idt[0x2C], intr_undefined0x2C);
  set_intr_gate(&idt[0x2D], intr_undefined0x2D);
  set_intr_gate(&idt[0x2E], intr_undefined0x2E);
  set_intr_gate(&idt[0x2F], intr_undefined0x2F);

  set_intr_gate(&idt[0x30], intr_undefined0x30);
  set_intr_gate(&idt[0x31], intr_undefined0x31);
  set_intr_gate(&idt[0x32], intr_undefined0x32);
  set_intr_gate(&idt[0x33], intr_undefined0x33);
  set_intr_gate(&idt[0x34], intr_undefined0x34);
  set_intr_gate(&idt[0x35], intr_undefined0x35);
  set_intr_gate(&idt[0x36], intr_undefined0x36);
  set_intr_gate(&idt[0x37], intr_undefined0x37);
  set_intr_gate(&idt[0x38], intr_undefined0x38);
  set_intr_gate(&idt[0x39], intr_undefined0x39);
  set_intr_gate(&idt[0x3A], intr_undefined0x3A);
  set_intr_gate(&idt[0x3B], intr_undefined0x3B);
  set_intr_gate(&idt[0x3C], intr_undefined0x3C);
  set_intr_gate(&idt[0x3D], intr_undefined0x3D);
  set_intr_gate(&idt[0x3E], intr_undefined0x3E);
  set_intr_gate(&idt[0x3F], intr_undefined0x3F);

  set_intr_gate(&idt[0x40], intr_undefined0x40);
  set_intr_gate(&idt[0x41], intr_undefined0x41);
  set_intr_gate(&idt[0x42], intr_undefined0x42);
  set_intr_gate(&idt[0x43], intr_undefined0x43);
  set_intr_gate(&idt[0x44], intr_undefined0x44);
  set_intr_gate(&idt[0x45], intr_undefined0x45);
  set_intr_gate(&idt[0x46], intr_undefined0x46);
  set_intr_gate(&idt[0x47], intr_undefined0x47);
  set_intr_gate(&idt[0x48], intr_undefined0x48);
  set_intr_gate(&idt[0x49], intr_undefined0x49);
  set_intr_gate(&idt[0x4A], intr_undefined0x4A);
  set_intr_gate(&idt[0x4B], intr_undefined0x4B);
  set_intr_gate(&idt[0x4C], intr_undefined0x4C);
  set_intr_gate(&idt[0x4D], intr_undefined0x4D);
  set_intr_gate(&idt[0x4E], intr_undefined0x4E);
  set_intr_gate(&idt[0x4F], intr_undefined0x4F);

  set_intr_gate(&idt[0x50], intr_undefined0x50);
  set_intr_gate(&idt[0x51], intr_undefined0x51);
  set_intr_gate(&idt[0x52], intr_undefined0x52);
  set_intr_gate(&idt[0x53], intr_undefined0x53);
  set_intr_gate(&idt[0x54], intr_undefined0x54);
  set_intr_gate(&idt[0x55], intr_undefined0x55);
  set_intr_gate(&idt[0x56], intr_undefined0x56);
  set_intr_gate(&idt[0x57], intr_undefined0x57);
  set_intr_gate(&idt[0x58], intr_undefined0x58);
  set_intr_gate(&idt[0x59], intr_undefined0x59);
  set_intr_gate(&idt[0x5A], intr_undefined0x5A);
  set_intr_gate(&idt[0x5B], intr_undefined0x5B);
  set_intr_gate(&idt[0x5C], intr_undefined0x5C);
  set_intr_gate(&idt[0x5D], intr_undefined0x5D);
  set_intr_gate(&idt[0x5E], intr_undefined0x5E);
  set_intr_gate(&idt[0x5F], intr_undefined0x5F);

  set_intr_gate(&idt[0x60], intr_undefined0x60);
  set_intr_gate(&idt[0x61], intr_undefined0x61);
  set_intr_gate(&idt[0x62], intr_undefined0x62);
  set_intr_gate(&idt[0x63], intr_undefined0x63);
  set_intr_gate(&idt[0x64], intr_undefined0x64);
  set_intr_gate(&idt[0x65], intr_undefined0x65);
  set_intr_gate(&idt[0x66], intr_undefined0x66);
  set_intr_gate(&idt[0x67], intr_undefined0x67);
  set_intr_gate(&idt[0x68], intr_undefined0x68);
  set_intr_gate(&idt[0x69], intr_undefined0x69);
  set_intr_gate(&idt[0x6A], intr_undefined0x6A);
  set_intr_gate(&idt[0x6B], intr_undefined0x6B);
  set_intr_gate(&idt[0x6C], intr_undefined0x6C);
  set_intr_gate(&idt[0x6D], intr_undefined0x6D);
  set_intr_gate(&idt[0x6E], intr_undefined0x6E);
  set_intr_gate(&idt[0x6F], intr_undefined0x6F);

  set_intr_gate(&idt[0x70], intr_undefined0x70);
  set_intr_gate(&idt[0x71], intr_undefined0x71);
  set_intr_gate(&idt[0x72], intr_undefined0x72);
  set_intr_gate(&idt[0x73], intr_undefined0x73);
  set_intr_gate(&idt[0x74], intr_undefined0x74);
  set_intr_gate(&idt[0x75], intr_undefined0x75);
  set_intr_gate(&idt[0x76], intr_undefined0x76);
  set_intr_gate(&idt[0x77], intr_undefined0x77);
  set_intr_gate(&idt[0x78], intr_undefined0x78);
  set_intr_gate(&idt[0x79], intr_undefined0x79);
  set_intr_gate(&idt[0x7A], intr_undefined0x7A);
  set_intr_gate(&idt[0x7B], intr_undefined0x7B);
  set_intr_gate(&idt[0x7C], intr_undefined0x7C);
  set_intr_gate(&idt[0x7D], intr_undefined0x7D);
  set_intr_gate(&idt[0x7E], intr_undefined0x7E);
  set_intr_gate(&idt[0x7F], intr_undefined0x7F);

  set_intr_gate(&idt[0x80], intr_undefined0x80);
  set_intr_gate(&idt[0x81], intr_undefined0x81);
  set_intr_gate(&idt[0x82], intr_undefined0x82);
  set_intr_gate(&idt[0x83], intr_undefined0x83);
  set_intr_gate(&idt[0x84], intr_undefined0x84);
  set_intr_gate(&idt[0x85], intr_undefined0x85);
  set_intr_gate(&idt[0x86], intr_undefined0x86);
  set_intr_gate(&idt[0x87], intr_undefined0x87);
  set_intr_gate(&idt[0x88], intr_undefined0x88);
  set_intr_gate(&idt[0x89], intr_undefined0x89);
  set_intr_gate(&idt[0x8A], intr_undefined0x8A);
  set_intr_gate(&idt[0x8B], intr_undefined0x8B);
  set_intr_gate(&idt[0x8C], intr_undefined0x8C);
  set_intr_gate(&idt[0x8D], intr_undefined0x8D);
  set_intr_gate(&idt[0x8E], intr_undefined0x8E);
  set_intr_gate(&idt[0x8F], intr_undefined0x8F);

  set_intr_gate(&idt[0x90], intr_undefined0x90);
  set_intr_gate(&idt[0x91], intr_undefined0x91);
  set_intr_gate(&idt[0x92], intr_undefined0x92);
  set_intr_gate(&idt[0x93], intr_undefined0x93);
  set_intr_gate(&idt[0x94], intr_undefined0x94);
  set_intr_gate(&idt[0x95], intr_undefined0x95);
  set_intr_gate(&idt[0x96], intr_undefined0x96);
  set_intr_gate(&idt[0x97], intr_undefined0x97);
  set_intr_gate(&idt[0x98], intr_undefined0x98);
  set_intr_gate(&idt[0x99], intr_undefined0x99);
  set_intr_gate(&idt[0x9A], intr_undefined0x9A);
  set_intr_gate(&idt[0x9B], intr_undefined0x9B);
  set_intr_gate(&idt[0x9C], intr_undefined0x9C);
  set_intr_gate(&idt[0x9D], intr_undefined0x9D);
  set_intr_gate(&idt[0x9E], intr_undefined0x9E);
  set_intr_gate(&idt[0x9F], intr_undefined0x9F);

  set_intr_gate(&idt[0xA0], intr_undefined0xA0);
  set_intr_gate(&idt[0xA1], intr_undefined0xA1);
  set_intr_gate(&idt[0xA2], intr_undefined0xA2);
  set_intr_gate(&idt[0xA3], intr_undefined0xA3);
  set_intr_gate(&idt[0xA4], intr_undefined0xA4);
  set_intr_gate(&idt[0xA5], intr_undefined0xA5);
  set_intr_gate(&idt[0xA6], intr_undefined0xA6);
  set_intr_gate(&idt[0xA7], intr_undefined0xA7);
  set_intr_gate(&idt[0xA8], intr_undefined0xA8);
  set_intr_gate(&idt[0xA9], intr_undefined0xA9);
  set_intr_gate(&idt[0xAA], intr_undefined0xAA);
  set_intr_gate(&idt[0xAB], intr_undefined0xAB);
  set_intr_gate(&idt[0xAC], intr_undefined0xAC);
  set_intr_gate(&idt[0xAD], intr_undefined0xAD);
  set_intr_gate(&idt[0xAE], intr_undefined0xAE);
  set_intr_gate(&idt[0xAF], intr_undefined0xAF);

  set_intr_gate(&idt[0xB0], intr_undefined0xB0);
  set_intr_gate(&idt[0xB1], intr_undefined0xB1);
  set_intr_gate(&idt[0xB2], intr_undefined0xB2);
  set_intr_gate(&idt[0xB3], intr_undefined0xB3);
  set_intr_gate(&idt[0xB4], intr_undefined0xB4);
  set_intr_gate(&idt[0xB5], intr_undefined0xB5);
  set_intr_gate(&idt[0xB6], intr_undefined0xB6);
  set_intr_gate(&idt[0xB7], intr_undefined0xB7);
  set_intr_gate(&idt[0xB8], intr_undefined0xB8);
  set_intr_gate(&idt[0xB9], intr_undefined0xB9);
  set_intr_gate(&idt[0xBA], intr_undefined0xBA);
  set_intr_gate(&idt[0xBB], intr_undefined0xBB);
  set_intr_gate(&idt[0xBC], intr_undefined0xBC);
  set_intr_gate(&idt[0xBD], intr_undefined0xBD);
  set_intr_gate(&idt[0xBE], intr_undefined0xBE);
  set_intr_gate(&idt[0xBF], intr_undefined0xBF);

  set_intr_gate(&idt[0xC0], intr_undefined0xC0);
  set_intr_gate(&idt[0xC1], intr_undefined0xC1);
  set_intr_gate(&idt[0xC2], intr_undefined0xC2);
  set_intr_gate(&idt[0xC3], intr_undefined0xC3);
  set_intr_gate(&idt[0xC4], intr_undefined0xC4);
  set_intr_gate(&idt[0xC5], intr_undefined0xC5);
  set_intr_gate(&idt[0xC6], intr_undefined0xC6);
  set_intr_gate(&idt[0xC7], intr_undefined0xC7);
  set_intr_gate(&idt[0xC8], intr_undefined0xC8);
  set_intr_gate(&idt[0xC9], intr_undefined0xC9);
  set_intr_gate(&idt[0xCA], intr_undefined0xCA);
  set_intr_gate(&idt[0xCB], intr_undefined0xCB);
  set_intr_gate(&idt[0xCC], intr_undefined0xCC);
  set_intr_gate(&idt[0xCD], intr_undefined0xCD);
  set_intr_gate(&idt[0xCE], intr_undefined0xCE);
  set_intr_gate(&idt[0xCF], intr_undefined0xCF);

  set_intr_gate(&idt[0xD0], intr_undefined0xD0);
  set_intr_gate(&idt[0xD1], intr_undefined0xD1);
  set_intr_gate(&idt[0xD2], intr_undefined0xD2);
  set_intr_gate(&idt[0xD3], intr_undefined0xD3);
  set_intr_gate(&idt[0xD4], intr_undefined0xD4);
  set_intr_gate(&idt[0xD5], intr_undefined0xD5);
  set_intr_gate(&idt[0xD6], intr_undefined0xD6);
  set_intr_gate(&idt[0xD7], intr_undefined0xD7);
  set_intr_gate(&idt[0xD8], intr_undefined0xD8);
  set_intr_gate(&idt[0xD9], intr_undefined0xD9);
  set_intr_gate(&idt[0xDA], intr_undefined0xDA);
  set_intr_gate(&idt[0xDB], intr_undefined0xDB);
  set_intr_gate(&idt[0xDC], intr_undefined0xDC);
  set_intr_gate(&idt[0xDD], intr_undefined0xDD);
  set_intr_gate(&idt[0xDE], intr_undefined0xDE);
  set_intr_gate(&idt[0xDF], intr_undefined0xDF);

  set_intr_gate(&idt[0xE0], intr_undefined0xE0);
  set_intr_gate(&idt[0xE1], intr_undefined0xE1);
  set_intr_gate(&idt[0xE2], intr_undefined0xE2);
  set_intr_gate(&idt[0xE3], intr_undefined0xE3);
  set_intr_gate(&idt[0xE4], intr_undefined0xE4);
  set_intr_gate(&idt[0xE5], intr_undefined0xE5);
  set_intr_gate(&idt[0xE6], intr_undefined0xE6);
  set_intr_gate(&idt[0xE7], intr_undefined0xE7);
  set_intr_gate(&idt[0xE8], intr_undefined0xE8);
  set_intr_gate(&idt[0xE9], intr_undefined0xE9);
  set_intr_gate(&idt[0xEA], intr_undefined0xEA);
  set_intr_gate(&idt[0xEB], intr_undefined0xEB);
  set_intr_gate(&idt[0xEC], intr_undefined0xEC);
  set_intr_gate(&idt[0xED], intr_undefined0xED);
  set_intr_gate(&idt[0xEE], intr_undefined0xEE);
  set_intr_gate(&idt[0xEF], intr_undefined0xEF);

  set_intr_gate(&idt[0xF0], intr_undefined0xF0);
  set_intr_gate(&idt[0xF1], intr_undefined0xF1);
  set_intr_gate(&idt[0xF2], intr_undefined0xF2);
  set_intr_gate(&idt[0xF3], intr_undefined0xF3);
  set_intr_gate(&idt[0xF4], intr_undefined0xF4);
  set_intr_gate(&idt[0xF5], intr_undefined0xF5);
  set_intr_gate(&idt[0xF6], intr_undefined0xF6);
  set_intr_gate(&idt[0xF7], intr_undefined0xF7);
  set_intr_gate(&idt[0xF8], intr_undefined0xF8);
  set_intr_gate(&idt[0xF9], intr_undefined0xF9);
  set_intr_gate(&idt[0xFA], intr_undefined0xFA);
  set_intr_gate(&idt[0xFB], intr_undefined0xFB);
  set_intr_gate(&idt[0xFC], intr_undefined0xFC);
  set_intr_gate(&idt[0xFD], intr_undefined0xFD);
  set_intr_gate(&idt[0xFE], intr_undefined0xFE);
  set_intr_gate(&idt[0xFF], intr_undefined0xFF);
}

AC_ATTR_INTR_HDLR
static void intr_divide_err(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#DE 0", frame);

  intr_divide_err_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_divide_err_counter: %d\n", intr_divide_err_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_debug(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#DB 1", frame);

  intr_debug_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_debug_counter: %d\n", intr_debug_counter);
  reset_x86();
}


AC_ATTR_INTR_HDLR
static void intr_nmi(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#NMI 2", frame);

  intr_nmi_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_nmi_counter: %d\n", intr_nmi_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_breakpoint(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#BP 3", frame);

  intr_breakpoint_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_breakpoint_counter: %d\n", intr_breakpoint_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_overflow(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#OF 4", frame);

  intr_overflow_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_overflow_counter: %d\n", intr_overflow_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_bound_range(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#BR 5", frame);

  intr_bound_range_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_bound_range_counter: %d\n", intr_bound_range_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_invalid_opcode(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#UD 6", frame);

  intr_invalid_opcode_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_invlid_opcode_counter: %d\n", intr_invalid_opcode_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_device_not_available(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#UD 7", frame);

  intr_device_not_available_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_device_not_available_counter: %d\n", intr_device_not_available_counter);
  reset_x86();
}


AC_ATTR_INTR_HDLR
static void expt_double_fault(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#DF 8", frame);

  expt_double_fault_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_double_fault_counter: %d\n", expt_double_fault_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_coprocessor_segment_overrun(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#CPSO 9", frame);

  intr_coprocessor_segment_overrun_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" coprocessor_segment_overrun_counter: %d\n", intr_coprocessor_segment_overrun_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void expt_invalid_tss(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#TS 10", frame);

  expt_invalid_tss_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_invalid_tss_counter: %d\n", expt_invalid_tss_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void expt_segment_not_present(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#NP 11", frame);

  expt_segment_not_present_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_segment_not_present_counter: %d\n", expt_segment_not_present_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void expt_stack_fault(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#SS 12", frame);

  expt_stack_fault_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_stack_fault_counter: %d\n", expt_stack_fault_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void expt_general_protection(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#GP13", frame);

  expt_general_protection_counter += 1;
  ac_printf(" expt_general_protection_counter: %d\n", expt_general_protection_counter);

  IdtPtr idtp;
  get_idt(&idtp);
  ac_printf(" idt.limit=%d\n", idtp.limit);
  ac_printf(" idt.iig=%p\n", idtp.iig);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void expt_page_fault(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#PF 14", frame);

  expt_page_fault_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_page_fault_counter: %d\n", expt_page_fault_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_x87_floating_point(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#MF 16", frame);

  intr_x87_floating_point_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_x87_floating_point_counter: %d\n", intr_x87_floating_point_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void expt_alignment_check(IntrFrame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#AC 17", frame);

  expt_alignment_check_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_alignment_check_counter: %d\n", expt_alignment_check_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_machine_check(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#MC 18", frame);

  intr_machine_check_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_machine_check_counter: %d\n", intr_machine_check_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_simd_floating_point(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#XM 19", frame);

  intr_simd_floating_point_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_simd_floating_point_counter: %d\n", intr_simd_floating_point_counter);
  reset_x86();
}

AC_ATTR_INTR_HDLR
static void intr_virtualization(IntrFrame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#VE 20", frame);

  intr_virtualization_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_virtualization_counter: %d\n", intr_virtualization_counter);
  reset_x86();
}

static void set_intr_gate(IntrGate* gate, intr_handler* ih) {
  static IntrGate g = INTR_GATE_INITIALIZER;
  *gate = g;

  gate->offset_lo = INTR_GATE_OFFSET_LO(ih);
  gate->offset_hi = INTR_GATE_OFFSET_HI(ih);
  gate->segment = 8; // Code Segment TI=0, RPL=0
                     // FIXME: How to know where the Code Segement is
#ifdef CPU_X86_64
  gate->ist = 0; // Modified legacy stack switching mode
#endif
  gate->type = DT_64_INTR_GATE;
  gate->dpl = 0;
  gate->p = 1;
}

static void set_expt_gate(IntrGate* gate, expt_handler* eh) {
  static IntrGate g = INTR_GATE_INITIALIZER;
  *gate = g;

  gate->offset_lo = INTR_GATE_OFFSET_LO(eh);
  gate->offset_hi = INTR_GATE_OFFSET_HI(eh);
  gate->segment = 8; // Code Segment TI=0, RPL=0
                     // FIXME: How to know where the Code Segement is
#ifdef CPU_X86_64
  gate->ist = 0; // Modified legacy stack switching mode
#endif
  gate->type = DT_64_INTR_GATE;
  gate->dpl = 0;
  gate->p = 1;
}

static void set_idtr(IntrGate idt[], ac_u32 count) {
  IdtPtr dp;
  dp.limit = (ac_u16)(((ac_uptr)&idt[count] - (ac_uptr)&idt[0] - 1)
      & 0xFFFF);
  dp.iig = &idt[0];
  set_idt(&dp);
}

void set_intr_handler(ac_u32 intr_num, intr_handler ih) {
  set_intr_gate(&idt[intr_num], ih);
}

void set_expt_handler(ac_u32 intr_num, expt_handler eh) {
  set_expt_gate(&idt[intr_num], eh);
}

IntrGate* get_intr_gate(ac_u32 intr_num) {
  return &idt[intr_num];
}

void initialize_intr_descriptor_table(void) {
  ac_printf("initialize_intr_descriptor_table:+\n");
  intr_divide_err_counter = 0;
  intr_debug_counter = 0;
  intr_nmi_counter = 0;
  intr_breakpoint_counter = 0;
  intr_overflow_counter = 0;
  intr_bound_range_counter = 0;
  intr_invalid_opcode_counter = 0;
  expt_double_fault_counter = 0;
  expt_invalid_tss_counter = 0;
  expt_segment_not_present_counter = 0;
  expt_stack_fault_counter = 0;
  expt_general_protection_counter = 0;
  expt_page_fault_counter = 0;
  intr_x87_floating_point_counter = 0;
  expt_alignment_check_counter = 0;
  intr_machine_check_counter = 0;
  intr_simd_floating_point_counter = 0;
  intr_virtualization_counter = 0;

  init_idt_to_undefined();

  // Initialize interrupts and exceptions
  set_intr_gate(&idt[0], intr_divide_err);
  set_intr_gate(&idt[1], intr_debug);
  set_intr_gate(&idt[2], intr_nmi);
  set_intr_gate(&idt[3], intr_breakpoint);
  set_intr_gate(&idt[4], intr_overflow);
  set_intr_gate(&idt[5], intr_bound_range);
  set_intr_gate(&idt[6], intr_invalid_opcode);
  set_intr_gate(&idt[7], intr_device_not_available);
  set_expt_gate(&idt[8], expt_double_fault);
  set_intr_gate(&idt[9], intr_coprocessor_segment_overrun);
  set_expt_gate(&idt[10], expt_invalid_tss);
  set_expt_gate(&idt[11], expt_segment_not_present);
  set_expt_gate(&idt[12], expt_stack_fault);
  set_expt_gate(&idt[13], expt_general_protection);
  set_expt_gate(&idt[14], expt_page_fault);

  set_intr_gate(&idt[16], intr_x87_floating_point);
  set_expt_gate(&idt[17], expt_alignment_check);
  set_intr_gate(&idt[18], intr_machine_check);
  set_intr_gate(&idt[19], intr_simd_floating_point);
  set_intr_gate(&idt[20], intr_virtualization);

  // set the idt register to point to idt array
  set_idtr(idt, AC_ARRAY_COUNT(idt));
  ac_printf("initialize_intr_descriptor_table:-\n");
}
