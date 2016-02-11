/*
 * Copyright 2015 Wink Saville
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

#include <msr_x86.h>
#include <msr_x86_print.h>

#include <ac_bits.h>
#include <ac_printf.h>
#include <ac_test.h>

ac_bool test_rdwrmsr() {
  ac_bool error = AC_FALSE;

  ac_u64 msr_apic_base = get_msr(MSR_APIC_BASE);
  print_msr(MSR_APIC_BASE, msr_apic_base);

  error |= AC_TEST(msr_apic_base != 0);

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_rdwrmsr();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
