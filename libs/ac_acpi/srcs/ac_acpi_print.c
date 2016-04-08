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

#include <ac_acpi.h>
#include <ac_acpi_print.h>

#include <ac_printf.h>

static void print_bytes(char* str, ac_u8* p, ac_uint length, char* terminator) {
  if (str != AC_NULL) {
    ac_printf(str);
  }
  for (ac_uint i = 0; i < length; i++) {
    ac_printf("%c", p[i]);
  }
  if (terminator != AC_NULL) {
    ac_printf(terminator);
  }
}

/**
 * Print a 4 byte descriptor ac_acpi_rsdp
 */
void ac_acpi_desc_hdr_signature_print(ac_u32 signature) {
  ac_u8* p = (ac_u8*)&signature;
  for (ac_uint i = 0; i < 4; i++) {
    ac_printf("%c", *p++);
  }
}

/**
 * Print ac_acpi_rsdp
 */
void ac_acpi_rsdp_print(char* str, ac_acpi_rsdp* rsdp) {
  if (str != AC_NULL) {
    ac_printf("%s ", str);
  }
  
  print_bytes("signature='", rsdp->signature, sizeof(rsdp->signature), "'");
  ac_printf(" check_sum=%d", rsdp->check_sum);
  print_bytes(" oem_id='", rsdp->oem_id, sizeof(rsdp->oem_id), "'");
  ac_printf(" revision=%d rsdt_address=%x",
    rsdp->revision, rsdp->rsdt_address);
  if (rsdp->revision == 2) {
    ac_printf(" length=%d xsdt_address=%lx extended_checksum=%d",
      rsdp->length, rsdp->xsdt_address, rsdp->extended_check_sum);
  }
  ac_printf("\n");

  ac_acpi_desc_hdr* sdt;
  if (rsdp->revision == 0) {
    sdt = (ac_acpi_desc_hdr*)(ac_uptr)rsdp->rsdt_address;
  } else if (rsdp->revision == 2) {
    sdt = (ac_acpi_desc_hdr*)(ac_uptr)rsdp->xsdt_address;
  } else {
    ac_printf("ac_acpi_rsdp_get: rdsp=%lx expected revision 0 or 2"
       " got revision=%d\n", rsdp, rsdp->revision);
    sdt = AC_NULL;
  }
  if (sdt != AC_NULL) {
    ac_printf("sdt=%p ", sdt);
    ac_acpi_desc_hdr_signature_print(sdt->signature);
    ac_printf("\n");
  }
}

/**
 * Print ac_acpi_rsdp
 */
void ac_acpi_desc_hdr_print(char* str, ac_acpi_desc_hdr* rsdp) {
}
