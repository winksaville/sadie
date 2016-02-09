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

#ifndef ARCH_X86_INCS_APIC_X86_H
#define ARCH_X86_INCS_APIC_X86_H

#include <ac_inttypes.h>

/**
 * Initialize APIC
 *
 * @return 0 if initialized, !0 if an error
 */
ac_uint initialize_apic();

/**
 * @return 0 if present, !0 if not
 */
ac_uint apic_present(void);

/**
 * @return id of the local apic
 */
ac_u32 apic_get_id(void);

/**
 * @return physical address of local apic
 */
ac_u64 apic_get_physical_addr(void);

#endif
