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

#ifndef SADIE_ARCH_ARMV6_LIBS_AC_ACPI_IMPL_INCS_AC_ACPI_IMPL_H
#define SADIE_ARCH_ARMV6_LIBS_AC_ACPI_IMPL_INCS_AC_ACPI_IMPL_H

#include <ac_inttypes.h>

/**
 * Get the acpi rsdp
 *
 * @return AC_NULL if rsdp does not exist
 */
static inline ac_acpi_rsdp* ac_acpi_rsdp_get(void) {
  // Not implemented
  return AC_NULL;
}

#endif
