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
 * Print ac_acpi_rsdp
 */
void ac_acpi_rsdp_print(char* str, ac_acpi_rsdp* rsdp) {
  if (str != AC_NULL) {
    ac_printf("%s", str);
  }
  
  print_bytes(" signature=", rsdp->signature, sizeof(rsdp->signature), "");
  ac_printf(" check_sum=%d", rsdp->check_sum);
  print_bytes(" oem_id= ", rsdp->oem_id, sizeof(rsdp->oem_id), "");
  ac_printf(" revision=%d rsdt_address=%x length=%d xsdt_address=%x extended_checksum=%d",
    rsdp->revision, rsdp->rsdt_address, rsdp->length, rsdp->xsdt_address, rsdp->extended_check_sum);
}
