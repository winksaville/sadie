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

#include <print_msr.h>

#include <msr_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

void print_msr_apic_base(ac_u64 value) {
  union msr_apic_base_u reg = { .raw = value };

  ac_printf("msr_apic_base 0x%x: 0x%lx\n", MSR_APIC_BASE, value);
  ac_printf(" reserved_0=%d\n", reg.fields.reserved_0);
  ac_printf(" bsp=%b\n", reg.fields.bsp);
  ac_printf(" reserved_1=%d\n", reg.fields.reserved_1);
  ac_printf(" extd=%b\n", reg.fields.extd);
  ac_printf(" e=%b\n", reg.fields.e);
  ac_printf(" base=0x%lx\n", msr_get_apic_base_physical_addr(reg.fields));
}
