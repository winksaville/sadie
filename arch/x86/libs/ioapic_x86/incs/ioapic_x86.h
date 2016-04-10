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

#ifndef ARCH_X86_LIBS_IOAPIC_X86_INCS_IOAPIC_X86_H
#define ARCH_X86_LIBS_IOAPIC_X86_INCS_IOAPIC_X86_H

#include <ac_inttypes.h>

struct ioapic_regs {
} __attribute__((__packed__));

typedef struct ioapic_regs ioapic_regs;

/**
 * @return the number of ioapic's.
 */
ac_uint ioapic_get_count(void);

/**
 * Get the address of an ioapic
 *
 * @param idx is the index to the ioapic 0..n where
 *        n is the value returned by ioapci_get_count().
 *
 * @return the address of an ioapic
 */
ioapic_regs* ioapic_get_addr(ac_uint idx);

#endif
