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

#include <page_table_x86.h>

#include <ac_check_sum.h>
#include <ac_inttypes.h>
#include <ac_memcmp.h>
#include <ac_printf.h>

#define BIOS_BEG_ADDR ((ac_acpi_rsdp*)0xE0000)
#define BIOS_END_ADDR ((ac_acpi_rsdp*)0xFFFFF)

/**
 * Get the acpi rsdp
 *
 * @return AC_NULL if rsdp does not exist
 */
ac_acpi_rsdp* ac_acpi_rsdp_get(void) {

#if 0
  // Map BIOS memory 1:1
  page_table_map_lin_to_phy(get_page_table_linear_addr(),
      BIOS_BEG_ADDR, (ac_u64)BIOS_BEG_ADDR,
      (((ac_u64)BIOS_END_ADDR) + 1) - (ac_u64)BIOS_BEG_ADDR,
      PAGE_CACHING_WRITE_BACK);
#endif

  ac_acpi_rsdp* rsdp;
  for (rsdp = BIOS_BEG_ADDR; rsdp <= BIOS_END_ADDR;
      rsdp = (ac_acpi_rsdp*)((ac_u8*)rsdp + 16)) {
    if (ac_memcmp(rsdp->signature, AC_ACPI_RSDP_SIGNATURE,
          sizeof(rsdp->signature)) == 0) {
      // Validate the structures check sum
      ac_u8 check_sum_length;
      ac_u64 sdt;
      ac_u8 rx_sdt;
      if (rsdp->revision == 0) {
        check_sum_length = 20;
        sdt = rsdp->rsdt_address;
        rx_sdt = 'r';
      } else if (rsdp->revision == 2) {
        check_sum_length = rsdp->length;
        sdt = rsdp->xsdt_address;
        rx_sdt = 'x';
      } else {
        ac_printf("ac_acpi_rsdp_get: rdsp=%lx expected revision 0 or 2"
           " got revision=%d\n", rsdp, rsdp->revision);
        continue;
      }
      ac_u8 check_sum = ac_check_sum_u8(rsdp, check_sum_length);
      if (check_sum != 0) {
        ac_printf("ac_acpi_rsdp_get: rdsp=%lx check_sum=%d != 0\n",
            rsdp, check_sum);
        continue;
      }

      ac_printf("ac_acpi_rsdp_get: Found rsdp=%lx revision=%d %csdt=%lx\n",
          rsdp, rsdp->revision, rx_sdt, sdt);
      return rsdp;
    }
  }

  return AC_NULL;
}
