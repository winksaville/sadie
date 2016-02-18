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

void print_msr_efer(ac_u64 value) {
  union msr_efer_u reg = { .raw = value };

  ac_printf("msr_efer 0x%x: 0x%lx\n", MSR_EFER, value);
  ac_printf(" sce=%d\n", reg.fields.sce);
  ac_printf(" reserved_0=%d\n", reg.fields.reserved_0);
  ac_printf(" lme=%d\n", reg.fields.lme);
  ac_printf(" reserved_1=%d\n", reg.fields.reserved_1);
  ac_printf(" lma=%b\n", reg.fields.lma);
  ac_printf(" nxe=%b\n", reg.fields.nxe);
  ac_printf(" reserved_2=0x%lx\n", reg.fields.reserved_2);
}
