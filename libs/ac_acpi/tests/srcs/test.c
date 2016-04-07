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
#include <ac_test.h>


int main(void) {
  ac_bool error = AC_FALSE;

  ac_acpi_rsdp* rsdp = ac_acpi_rsdp_get();
  if (rsdp == AC_NULL) {
    ac_printf("ac_acpi_rsdp does not exist, skipping tests\n");
  } else {
    ac_acpi_rsdp_print("rsdp: ", rsdp);
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
