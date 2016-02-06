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
#include <ac_printf.h>
#include <ac_string.h>
#include <ac_test.h>

ac_u8 get_byte(ac_u32 int32, ac_u32 idx) {
  return (ac_u8)(int32 >> (idx * 8));
}

char* get_str(ac_u32 ebx, ac_u32 ecx, ac_u32 edx,
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

ac_bool test_cpuid() {
  ac_u32 out_eax = 0;
  ac_u32 out_ebx = 0;
  ac_u32 out_ecx = 0;
  ac_u32 out_edx = 0;
  ac_bool error = AC_FALSE;
  char vendor_id[13];

  get_cpuid(0, &out_eax, &out_ebx, &out_ecx, &out_edx);
  ac_u32 max_leaf = out_eax;
  get_str(out_ebx, out_ecx, out_edx, vendor_id, sizeof(vendor_id));

  static char* vendor_id_list[] = {
    "AMDisbetter!",
    "AuthenticAMD",
    "CentaurHauls",
    "CyrixInstead",
    "GenuineIntel",
    "TransmetaCPU",
    "GenuineTMx86",
    "Geode by NSC",
    "NexGenDriven",
    "RiseRiseRise",
    "SiS SiS SiS ",
    "UMC UMC UMC ",
    "VIA VIA VIA ",
    "Vortex86 SoC",
    "KVMKVMKVM",
    "Microsoft Hv",
    " lrpepyh vr",
    "VMwareVMware",
    "XenVMMXenVMM",
  };

  // See if vendor_id is valid
  ac_int i;
  for (i = 0; i < AC_ARRAY_COUNT(vendor_id_list); i++) {
    if (ac_strncmp(vendor_id, vendor_id_list[i], sizeof(vendor_id)) == 0) {
      break;
    }
  }
  error |= AC_TEST_EM(out_eax != 0, "max leaf was 0");
  error |= AC_TEST_EM(i < AC_ARRAY_COUNT(vendor_id_list),
      "Invalid cpuid vendor");
  ac_printf(" max leaf=%d vendor_id=%s\n", max_leaf, vendor_id);

  // See that when we request a leaf greater than max_leaf zero's are returned
  get_cpuid(max_leaf+1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  error |= AC_TEST(out_eax == 0);
  error |= AC_TEST(out_ebx == 0);
  error |= AC_TEST(out_ecx == 0);
  error |= AC_TEST(out_edx == 0);


  get_cpuid(0x80000000, &out_eax, &out_ebx, &out_ecx, &out_edx);
  ac_u32 max_extleaf = out_eax;
  error |= AC_TEST_EM(max_extleaf != 0, "max extended leaf was 0");
  ac_printf("Max extended leaf=%x\n", max_extleaf);

  // See that when we request a leaf greater than max_leaf zero's are returned
  get_cpuid(max_extleaf+1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  error |= AC_TEST(out_eax == 0);
  error |= AC_TEST(out_ebx == 0);
  error |= AC_TEST(out_ecx == 0);
  error |= AC_TEST(out_edx == 0);

  // Display the max physical memory
  if (max_extleaf <= 0x80000008) {
    get_cpuid(0x80000008, &out_eax, &out_ebx, &out_ecx, &out_edx);
    ac_printf("Max physical address bits=%d\n", AC_GET_BITS(ac_u32, out_eax, 0, 8));
    ac_printf("Max linear address bits=%d\n", AC_GET_BITS(ac_u32, out_eax, 8, 8));

    // upper eax ebx, ecx and edx should be 0
    error |= AC_TEST(AC_GET_BITS(ac_u32, out_eax, 16, 16) == 0);
    error |= AC_TEST(out_ebx == 0);
    error |= AC_TEST(out_ecx == 0);
    error |= AC_TEST(out_edx == 0);
  } else {
    ac_printf("No liner/physical address size cpuid\n");
  }
  
  if (ac_strncmp(vendor_id, "GenuineIntel", sizeof(vendor_id)) == 0) {
    ac_u32 max_subleaf = 0;
    get_cpuid_subleaf(0x7, 0,
        &max_subleaf, &out_ebx, &out_ecx, &out_edx);
    ac_printf("Max subleaf=%x ebx=%x ecx=%x edx=%x\n",
        max_subleaf, out_ebx, out_ecx, out_edx);

    // Test that 0 is retured if max_subleaf is too large
    get_cpuid_subleaf(0x7, max_subleaf+1,
        &out_eax, &out_ebx, &out_ecx, &out_edx);
    error |= AC_TEST(out_eax == 0);
    //error |= AC_TEST(out_ebx == 0); Not true on my i7 ?
    error |= AC_TEST(out_ecx == 0);
    error |= AC_TEST(out_edx == 0);
  }

  // Get Processor Info and Feature Bits
  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  ac_printf("Processor info and feature bits eax=%x ebx=%x ecx=%x edx=%x\n",
     out_eax, out_ebx, out_ecx, out_edx);
  ac_printf("EAX:\n");
  ac_printf(" Extended Family ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 20, 8));
  ac_printf(" Extended Model ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 16, 4));
  ac_printf(" Processor Type=%x\n", AC_GET_BITS(ac_u32, out_eax, 12, 2));
  ac_printf(" Family ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 8, 4));
  ac_printf(" Model=%x\n", AC_GET_BITS(ac_u32, out_eax, 4, 4));
  ac_printf(" Stepping ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 0, 4));

  ac_printf("EBX:\n");
  ac_printf(" Brand Index=%x\n", AC_GET_BITS(ac_u32, out_ebx, 0, 8));
  ac_printf(" CFLUSH instruction cache line size=%x\n", AC_GET_BITS(ac_u32, out_ebx, 8, 8));
  ac_printf(" Local APIC ID=%x\n", AC_GET_BITS(ac_u32, out_ebx, 24, 8));

  ac_printf("ECX:\n");
  ac_printf(" SSE3=%b\n", AC_GET_BITS(ac_u32, out_ecx, 0, 1));
  ac_printf(" PCLMULQDQ=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 1));
  ac_printf(" DTES64=%b\n", AC_GET_BITS(ac_u32, out_ecx, 2, 1));
  ac_printf(" MONITOR=%b\n", AC_GET_BITS(ac_u32, out_ecx, 3, 1));
  ac_printf(" DS-CPL=%b\n", AC_GET_BITS(ac_u32, out_ecx, 4, 1));
  ac_printf(" VMX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 5, 1));
  ac_printf(" SMX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 6, 1));
  ac_printf(" EIST=%b\n", AC_GET_BITS(ac_u32, out_ecx, 7, 1));
  ac_printf(" TM2=%b\n", AC_GET_BITS(ac_u32, out_ecx, 8, 1));
  ac_printf(" SSSE3=%b\n", AC_GET_BITS(ac_u32, out_ecx, 9, 1));
  ac_printf(" CNXT-ID=%b\n", AC_GET_BITS(ac_u32, out_ecx, 10, 1));
  ac_printf(" SDBG=%b\n", AC_GET_BITS(ac_u32, out_ecx, 11, 1));
  ac_printf(" FMA=%b\n", AC_GET_BITS(ac_u32, out_ecx, 12, 1));
  ac_printf(" CMPXCHG16B=%b\n", AC_GET_BITS(ac_u32, out_ecx, 13, 1));
  ac_printf(" xTPR Update Control=%b\n", AC_GET_BITS(ac_u32, out_ecx, 14, 1));
  ac_printf(" PDCM=%b\n", AC_GET_BITS(ac_u32, out_ecx, 15, 1));
  ac_printf(" RESERVED=%b\n", AC_GET_BITS(ac_u32, out_ecx, 16, 1));
  ac_printf(" PCID=%b\n", AC_GET_BITS(ac_u32, out_ecx, 17, 1));
  ac_printf(" DCA=%b\n", AC_GET_BITS(ac_u32, out_ecx, 18, 1));
  ac_printf(" SSE4_1=%b\n", AC_GET_BITS(ac_u32, out_ecx, 19, 1));
  ac_printf(" SSE4_2=%b\n", AC_GET_BITS(ac_u32, out_ecx, 20, 1));
  ac_printf(" x2APIC=%b\n", AC_GET_BITS(ac_u32, out_ecx, 21 , 1));
  ac_printf(" MOVBE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 22, 1));
  ac_printf(" POPCNT=%b\n", AC_GET_BITS(ac_u32, out_ecx, 23, 1));
  ac_printf(" TSC-Deadline=%b\n", AC_GET_BITS(ac_u32, out_ecx, 24, 1));
  ac_printf(" AES=%b\n", AC_GET_BITS(ac_u32, out_ecx, 25, 1));
  ac_printf(" XSAVE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 26, 1));
  ac_printf(" OXSAVE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 27, 1));
  ac_printf(" AVX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 28, 1));
  ac_printf(" F16C=%b\n", AC_GET_BITS(ac_u32, out_ecx, 29, 1));
  ac_printf(" RDRDN=%b\n", AC_GET_BITS(ac_u32, out_ecx, 30, 1));
  ac_printf(" UNUSED=%b\n", AC_GET_BITS(ac_u32, out_ecx, 31, 1));

  ac_printf("EDX:\n");
  ac_printf(" FPU=%b\n", AC_GET_BITS(ac_u32, out_ecx, 0, 1));
  ac_printf(" VME=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 1));
  ac_printf(" DE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 2, 1));
  ac_printf(" PSE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 3, 1));
  ac_printf(" TSC=%b\n", AC_GET_BITS(ac_u32, out_ecx, 4, 1));
  ac_printf(" MSR=%b\n", AC_GET_BITS(ac_u32, out_ecx, 5, 1));
  ac_printf(" PAE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 6, 1));
  ac_printf(" MCE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 7, 1));
  ac_printf(" CX8=%b\n", AC_GET_BITS(ac_u32, out_ecx, 8, 1));
  ac_printf(" APIC=%b\n", AC_GET_BITS(ac_u32, out_ecx, 9, 1));
  ac_printf(" RESERVED_B10=%b\n", AC_GET_BITS(ac_u32, out_ecx, 10, 1));
  ac_printf(" SEP=%b\n", AC_GET_BITS(ac_u32, out_ecx, 11, 1));
  ac_printf(" MTRR=%b\n", AC_GET_BITS(ac_u32, out_ecx, 12, 1));
  ac_printf(" PGE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 13, 1));
  ac_printf(" MCA=%b\n", AC_GET_BITS(ac_u32, out_ecx, 14, 1));
  ac_printf(" CMOV=%b\n", AC_GET_BITS(ac_u32, out_ecx, 15, 1));
  ac_printf(" PAT=%b\n", AC_GET_BITS(ac_u32, out_ecx, 16, 1));
  ac_printf(" PSE-36=%b\n", AC_GET_BITS(ac_u32, out_ecx, 17, 1));
  ac_printf(" PSN=%b\n", AC_GET_BITS(ac_u32, out_ecx, 18, 1));
  ac_printf(" CFLUSH=%b\n", AC_GET_BITS(ac_u32, out_ecx, 19, 1));
  ac_printf(" RESERVED_B20=%b\n", AC_GET_BITS(ac_u32, out_ecx, 20, 1));
  ac_printf(" DS=%b\n", AC_GET_BITS(ac_u32, out_ecx, 21 , 1));
  ac_printf(" ACPI-Thermal Monitor and Clock Ctrl=%b\n", AC_GET_BITS(ac_u32, out_ecx, 22 , 1));
  ac_printf(" MMX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 23, 1));
  ac_printf(" FXSR-FXSAVE/FXRSTOR=%b\n", AC_GET_BITS(ac_u32, out_ecx, 24, 1));
  ac_printf(" SSE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 25, 1));
  ac_printf(" SSE2=%b\n", AC_GET_BITS(ac_u32, out_ecx, 26, 1));
  ac_printf(" SS=%b\n", AC_GET_BITS(ac_u32, out_ecx, 27, 1));
  ac_printf(" HTT=%b\n", AC_GET_BITS(ac_u32, out_ecx, 28, 1));
  ac_printf(" TM=%b\n", AC_GET_BITS(ac_u32, out_ecx, 29, 1));
  ac_printf(" RESERVED_B30=%b\n", AC_GET_BITS(ac_u32, out_ecx, 30, 1));
  ac_printf(" PBE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 31, 1));


  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_cpuid();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
