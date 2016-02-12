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

#include <msr_mtrrcap_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

void print_msr_mtrrcap(ac_u64 value) {
  union msr_mtrrcap_u reg = { .raw = value };

  ac_printf("msr_mtrrcap 0x%x: 0x%llx\n", MSR_MTRRCAP, value);
  ac_printf(" vcnt=%d\n", reg.fields.vcnt);
  ac_printf(" fix=%d\n", reg.fields.fix);
  ac_printf(" reserved_0=%d\n", reg.fields.reserved_0);
  ac_printf(" wc=%d\n", reg.fields.wc);
  ac_printf(" smrr=%d\n", reg.fields.smrr);
  ac_printf(" wc=%b\n", reg.fields.wc);
  ac_printf(" reserved_1=%llx\n", reg.fields.reserved_1);
}

