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
#include <ac_memcpy.h>

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
void ac_acpi_desc_hdr_identifier_print(char* str, ac_u32 identifier, char* terminator) {
  print_bytes(str, (ac_u8*)&identifier, sizeof(identifier), terminator);
}

/**
 * Print descriptor signature and lenght
 */
void ac_acpi_desc_hdr_sig_len_print(char* str, ac_acpi_desc_hdr* hdr) {
  if (str != AC_NULL) {
    ac_printf("%s", str);
  }

  ac_acpi_desc_hdr_identifier_print("sig='", hdr->signature,  "'");
  ac_printf(" len=%d", hdr->length);
}

/**
 * Print ac_acpi_rsdp
 */
void ac_acpi_desc_hdr_print(char* str, ac_acpi_desc_hdr* hdr) {
  if (str != AC_NULL) {
    ac_printf("%s", str);
  }

  ac_acpi_desc_hdr_sig_len_print("sig=", hdr);
  ac_printf(" rev=%d", hdr->revision);
  ac_printf(" cs=%d", hdr->check_sum);
  print_bytes(" oid='", hdr->oem_id, sizeof(hdr->oem_id), "'");
  print_bytes(" otid='", hdr->oem_table_id, sizeof(hdr->oem_table_id), "'");
  ac_printf(" orev=%d", hdr->oem_revision);
  ac_acpi_desc_hdr_identifier_print(" cid='", hdr->creator_id,  "'");
  ac_printf(" crev=%d", hdr->creator_revision);
  ac_printf("\n");
}

/**
 * Print ac_acpi_rsdp
 */
void ac_acpi_rsdp_print(char* str, ac_acpi_rsdp* rsdp) {
  if (str != AC_NULL) {
    ac_printf("%s ", str);
  }
  
  print_bytes("sig='", rsdp->signature, sizeof(rsdp->signature), "'");
  ac_printf(" cs=%d", rsdp->check_sum);
  print_bytes(" oid='", rsdp->oem_id, sizeof(rsdp->oem_id), "'");
  ac_printf(" rev=%d", rsdp->revision);
  ac_printf(" rsdt=%x", rsdp->rsdt_address);
  if (rsdp->revision == 2) {
    ac_printf(" len=%d xsdt=%lx xcs=%d",
      rsdp->length, rsdp->xsdt_address, rsdp->extended_check_sum);
  }
  ac_printf("\n");

  ac_acpi_desc_hdr* sdt;
  ac_uint sdt_entry_size;
  if (rsdp->revision == 0) {
    sdt = (ac_acpi_desc_hdr*)(ac_uptr)rsdp->rsdt_address;
    sdt_entry_size = sizeof(ac_u32);
  } else if (rsdp->revision == 2) {
    sdt = (ac_acpi_desc_hdr*)(ac_uptr)rsdp->xsdt_address;
    sdt_entry_size = sizeof(ac_u64);
  } else {
    ac_printf("ac_acpi_rsdp_get:-rdsp=%lx expected revision 0 or 2"
       " got revision=%d\n", rsdp, rsdp->revision);
    return;
  }

  if (sdt != AC_NULL) {
    ac_printf("sdt=%p ", sdt);
    ac_acpi_desc_hdr_sig_len_print("", sdt);
    ac_printf("\n");
    for (ac_uint i = 0; i < sdt->length - sizeof(ac_acpi_desc_hdr); i += sdt_entry_size) {
      ac_acpi_desc_hdr* pentry = AC_NULL;
      ac_memcpy(&pentry, &sdt->table[i], sdt_entry_size);
      ac_printf("pentry=%p", pentry);
      ac_acpi_desc_hdr_print(" ", pentry);
    }
  }
}
