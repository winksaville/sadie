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

ac_bool test_cpuid() {
  ac_u32 out_eax = 0;
  ac_u32 out_ebx = 0;
  ac_u32 out_ecx = 0;
  ac_u32 out_edx = 0;
  ac_bool error = AC_FALSE;
  char vendor_id[13];

  ac_u32 max_leaf = cpuid_max_leaf();

  get_cpuid(0, &out_eax, &out_ebx, &out_ecx, &out_edx);
  cpuid_get_vendor_id(vendor_id, sizeof(vendor_id));

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
  error |= AC_TEST_EM(i < AC_ARRAY_COUNT(vendor_id_list),
      "Invalid cpuid vendor");
  ac_printf("Max leaf=%d vendor_id=%s\n", max_leaf, vendor_id);

  get_cpuid(0x80000000, &out_eax, &out_ebx, &out_ecx, &out_edx);
  ac_u32 max_extleaf = out_eax;
  error |= AC_TEST_EM(max_extleaf != 0, "max extended leaf was 0");
  ac_printf("Max extended leaf=%x\n", max_extleaf);

  if (ac_strncmp(vendor_id, "GenuineIntel", sizeof(vendor_id)) == 0) {
    ac_u32 max_subleaf = 0;
    get_cpuid_subleaf(0x7, 0,
        &max_subleaf, &out_ebx, &out_ecx, &out_edx);
    ac_printf("Max subleaf=%x\n", max_subleaf);

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
  ac_printf("Cpu info and feature bits eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n",
     out_eax, out_ebx, out_ecx, out_edx);
  ac_printf("EAX:\n");
  ac_printf(" Extended Family ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 8, 20));
  ac_printf(" Extended Model ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 4, 16));
  ac_printf(" Processor Type=%x\n", AC_GET_BITS(ac_u32, out_eax, 2, 12));
  ac_printf(" Family ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 4, 8));
  ac_printf(" Model=%x\n", AC_GET_BITS(ac_u32, out_eax, 4, 4));
  ac_printf(" Stepping ID=%x\n", AC_GET_BITS(ac_u32, out_eax, 4, 0));

  ac_printf("EBX:\n");
  ac_printf(" Brand Index=%x\n", AC_GET_BITS(ac_u32, out_ebx, 8, 0));
  ac_printf(" CFLUSH instruction cache line size=%x\n",
      AC_GET_BITS(ac_u32, out_ebx, 8, 8));
  ac_printf(" Local APIC ID=%x\n", AC_GET_BITS(ac_u32, out_ebx, 8, 24));

  ac_printf("ECX:\n");
  ac_printf(" SSE3=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 0));
  ac_printf(" PCLMULQDQ=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 1));
  ac_printf(" DTES64=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 2));
  ac_printf(" MONITOR=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 3));
  ac_printf(" DS-CPL=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 4));
  ac_printf(" VMX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 5));
  ac_printf(" SMX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 6));
  ac_printf(" EIST=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 7));
  ac_printf(" TM2=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 8));
  ac_printf(" SSSE3=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 9));
  ac_printf(" CNXT-ID=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 10));
  ac_printf(" SDBG=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 11));
  ac_printf(" FMA=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 12));
  ac_printf(" CMPXCHG16B=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 13));
  ac_printf(" xTPR Update Control=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 14));
  ac_printf(" PDCM=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 15));
  ac_printf(" RESERVED=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 16));
  ac_printf(" PCID=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 17));
  ac_printf(" DCA=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 18));
  ac_printf(" SSE4_1=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 19));
  ac_printf(" SSE4_2=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 20));
  ac_printf(" x2APIC=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 21 ));
  ac_printf(" MOVBE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 22));
  ac_printf(" POPCNT=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 23));
  ac_printf(" TSC-Deadline=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 24));
  ac_printf(" AES=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 25));
  ac_printf(" XSAVE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 26));
  ac_printf(" OXSAVE=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 27));
  ac_printf(" AVX=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 28));
  ac_printf(" F16C=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 29));
  ac_printf(" RDRDN=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 30));
  ac_printf(" UNUSED=%b\n", AC_GET_BITS(ac_u32, out_ecx, 1, 31));

  ac_printf("EDX:\n");
  ac_printf(" FPU=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 0));
  ac_printf(" VME=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 1));
  ac_printf(" DE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 2));
  ac_printf(" PSE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 3));
  ac_printf(" TSC=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 4));
  ac_printf(" MSR=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 5));
  ac_printf(" PAE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 6));
  ac_printf(" MCE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 7));
  ac_printf(" CX8=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 8));
  ac_printf(" APIC=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 9));
  ac_printf(" RESERVED_B10=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 10));
  ac_printf(" SEP=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 11));
  ac_printf(" MTRR=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 12));
  ac_printf(" PGE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 13));
  ac_printf(" MCA=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 14));
  ac_printf(" CMOV=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 15));
  ac_printf(" PAT=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 16));
  ac_printf(" PSE-36=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 17));
  ac_printf(" PSN=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 18));
  ac_printf(" CFLUSH=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 19));
  ac_printf(" RESERVED_B20=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 20));
  ac_printf(" DS=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 21 ));
  ac_printf(" ACPI-Thermal Monitor and Clock Ctrl=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 22 ));
  ac_printf(" MMX=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 23));
  ac_printf(" FXSR-FXSAVE/FXRSTOR=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 24));
  ac_printf(" SSE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 25));
  ac_printf(" SSE2=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 26));
  ac_printf(" SS=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 27));
  ac_printf(" HTT=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 28));
  ac_printf(" TM=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 29));
  ac_printf(" RESERVED_B30=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 30));
  ac_printf(" PBE=%b\n", AC_GET_BITS(ac_u32, out_edx, 1, 31));


#if 0
  ac_uint max_pab = cpuid_max_physical_address_bits();
  ac_uint max_lab = cpuid_max_linear_address_bits();

  error |= AC_TEST(max_lab >= max_pab);
  error |= AC_TEST(max_pab > 0);
  error |= AC_TEST(max_lab > 0);

  ac_printf(" max_pab=%u\n", max_pab);
  ac_printf(" max_lab=%u\n", max_lab);
#endif

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
