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

#include <cpuid_x86.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_putchar.h>

static ac_u8 get_byte(ac_u32 int32, ac_u32 idx) {
  return (ac_u8)(int32 >> (idx * 8));
}

static char* get_str(ac_u32 ebx, ac_u32 ecx, ac_u32 edx,
    char* s, ac_int sizeof_s) {
  ac_int i;
  if (sizeof_s <= 0) {
    // Should never happen
    return AC_NULL;
  }

  for (i = 0; i < 4; i++) {
    if (i < sizeof_s - 1) {
      s[i] = get_byte(ebx, i);
    }
    if ((8+i) < sizeof_s - 1) {
      s[8+i] = get_byte(ecx, i);
    }
    if ((4+i) < sizeof_s - 1) {
      s[4+i] = get_byte(edx, i);
    }
  }
  s[sizeof_s - 1] = 0;

  return s;
}


void cpuid_get_vendor_id(char* vendor_id, ac_uint size_vendor_id) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(0, &out_eax, &out_ebx, &out_ecx, &out_edx);
  get_str(out_ebx, out_ecx, out_edx, vendor_id, size_vendor_id);
}

ac_u32 cpuid_max_leaf(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(0, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return out_eax;
}

ac_u32 cpuid_max_extd_leaf(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(0x80000000, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return out_eax;
}

ac_u32 cpuid_max_physical_address_bits() {
  if(cpuid_max_extd_leaf() >= 0x80000008) {
    ac_u32 out_eax, out_ebx, out_ecx, out_edx;
    get_cpuid(0x80000008, &out_eax, &out_ebx, &out_ecx, &out_edx);
    return AC_GET_BITS(ac_u32, out_eax, 0, 8);
  } else {
    // As per "Intel 64 and IA-32 Architectures Software Devemoper's Manual, Volume 3A"
    // Chapter 10.4.4 Local APIC Status and Location" foot note 1.
    return 36;
  }
}

ac_u32 cpuid_max_linear_address_bits() {
  if(cpuid_max_extd_leaf() >= 0x80000008) {
    ac_u32 out_eax, out_ebx, out_ecx, out_edx;
    get_cpuid(0x80000008, &out_eax, &out_ebx, &out_ecx, &out_edx);
    return AC_GET_BITS(ac_uint, out_eax, 8, 8);
  } else {
    return cpuid_max_physical_address_bits();
  }
}
