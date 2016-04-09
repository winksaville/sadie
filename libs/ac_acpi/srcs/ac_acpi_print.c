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

#include <ac_check_sum.h>
#include <ac_memcmp.h>
#include <ac_memcpy.h>

#include <ac_printf.h>

static void print_ascii_bytes(char* str, void* p, ac_uint length, char* terminator) {
  if (str != AC_NULL) {
    ac_printf(str);
  }
  ac_u8* pu8 = (ac_u8*)p;
  for (ac_uint i = 0; i < length; i++) {
    if (pu8[i] == 0) {
      break;
    }
    ac_printf("%c", pu8[i]);
  }
  if (terminator != AC_NULL) {
    ac_printf(terminator);
  }
}

__attribute__((__unused__))
static void print_hex_bytes(char* str, void* p, ac_uint length, char* terminator) {
  if (str != AC_NULL) {
    ac_printf(str);
  }
  ac_printf("p=%p l=%d ", p, length);
  ac_u8* pu8 = (ac_u8*)p;
  for (ac_uint i = 0; i < length; i++) {
    ac_printf("%x ", pu8[i]);
  }
  if (terminator != AC_NULL) {
    ac_printf(terminator);
  }
}

/**
 * Print descriptor signature and lenght
 */
void ac_acpi_desc_hdr_sig_len_print(char* str, ac_acpi_desc_hdr* hdr) {
  if (str != AC_NULL) {
    ac_printf(str);
  }

  print_ascii_bytes("sig='", hdr->signature, sizeof(hdr->signature), "'");
  ac_printf(" len=%d", hdr->length);
}

/**
 * Print ac_acpi_rsdp
 */
void ac_acpi_desc_hdr_print(char* str, ac_acpi_desc_hdr* hdr, char* terminator) {
  if (str != AC_NULL) {
    ac_printf(str);
  }

  ac_acpi_desc_hdr_sig_len_print(" ", hdr);
  ac_printf(" rev=%d", hdr->revision);
  ac_printf(" cs=%d", hdr->check_sum);
  print_ascii_bytes(" oid='", hdr->oem_id, sizeof(hdr->oem_id), "'");
  print_ascii_bytes(" otid='", hdr->oem_table_id, sizeof(hdr->oem_table_id), "'");
  ac_printf(" orev=%d", hdr->oem_revision);
  print_ascii_bytes(" cid='", hdr->creator_id, sizeof(hdr->creator_id), "'");
  ac_printf(" crev=%d", hdr->creator_revision);
  if (terminator != AC_NULL) {
    ac_printf(terminator);
  }
}

void ac_acpi_madt_print(char* str, ac_acpi_desc_hdr* hdr) {
  ac_acpi_madt* pmadt = (ac_acpi_madt*)hdr;
  if (str != AC_NULL) {
    ac_printf(str);
  }
  ac_acpi_desc_hdr_print("APIC: ", &pmadt->hdr, " ");
  ac_printf("addr=%x pcat_comp=%d\n", pmadt->addr, pmadt->pcat_comp);
  ac_acpi_madt_tl* pentry;
  ac_uint table_len = pmadt->hdr.length - sizeof(ac_acpi_madt);
  for (ac_uint i = 0; i < table_len; i += pentry->len) {
    pentry = (ac_acpi_madt_tl*)&pmadt->table[i];
    ac_printf(" madt: type=%d len=%d", pentry->type, pentry->len);
    if (pentry->len == 0) {
      ac_printf(" 0 is a bad length\n");
      break;
    }
    switch (pentry->type) {
      case MADT_TYPE_LOCAL_APIC: {
        ac_acpi_madt_local_apic* p = (ac_acpi_madt_local_apic*)pentry;
        ac_printf(" LOCAL_APIC proc_uid=%d id=%d enabled=%d\n",
            p->proc_uid, p->id, p->enabled);
        break;
      }
      case MADT_TYPE_IO_APIC: {
        ac_acpi_madt_io_apic* p = (ac_acpi_madt_io_apic*)pentry;
        ac_printf(" IO_APIC id=%d addr=%x global_system_intr_base=%d\n",
            p->id, p->addr, p->global_system_intr_base);
        break;
      }
      case MADT_TYPE_INTR_SOURCE_OVERRIDE: {
        ac_acpi_madt_intr_source_override * p = (ac_acpi_madt_intr_source_override*)pentry;
        ac_printf(" INTR_SOURCE_OVERRIDE bus=%d source=%x global_system_intr=%d"
            " polarity=%d trigger_mode=%d\n",
            p->bus, p->source, p->global_system_intr, p->polarity, p->trigger_mode);
        break;
      }
      case MADT_TYPE_LOCAL_APIC_NMI: {
        ac_acpi_madt_local_apic_nmi* p = (ac_acpi_madt_local_apic_nmi*)pentry;
        ac_printf(" LOCAL_APIC_NMI proc_uid=%d polarity=%x trigger_mode=%d lint=%d\n",
            p->proc_uid, p->polarity, p->trigger_mode, p->lint);
        break;
      }
      default: {
        ac_printf("madt: unknown type=%d\n", pentry->type);
        break;
      }
    }
  }
}

/**
 * Print ac_acpi_rsdp
 */
void ac_acpi_rsdp_print(char* str, ac_acpi_rsdp* rsdp) {
  if (str != AC_NULL) {
    ac_printf("%s ", str);
  }
  
  print_ascii_bytes("sig='", rsdp->signature, sizeof(rsdp->signature), "'");
  ac_printf(" cs=%d", rsdp->check_sum);
  print_ascii_bytes(" oid='", rsdp->oem_id, sizeof(rsdp->oem_id), "'");
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
      ac_u8 check_sum = ac_check_sum_u8(pentry, pentry->length);
      ac_printf("pentry=%p ", pentry);
      if (check_sum == 0) {
        if (ac_memcmp(&pentry->signature, "APIC", sizeof(pentry->signature)) == 0) {
          ac_acpi_madt_print("", pentry);
        } else {
          ac_acpi_desc_hdr_print("XXXX: ", pentry, "\n");
        }
      } else {
        ac_printf(" sum=%d is NOT zero, skipping\n", pentry, check_sum);
      }
    }
  }
}
