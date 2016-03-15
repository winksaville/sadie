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

#include <test_msr_x86.h>

#include <interrupts_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_msr_apic_base_fields();
  error |= test_msr_efer_fields();
  error |= test_msr_mtrrcap_fields();

  test_msr_perf_power();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
