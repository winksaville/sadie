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

#ifndef ARCH_X86_MSR_X86_TESTS_INCS_TEST_MSR_X86_H
#define ARCH_X86_MSR_X86_TESTS_INCS_TEST_MSR_X86_H

#include <ac_inttypes.h>

ac_bool test_msr_apic_base_fields();

ac_bool test_msr_efer_fields();

ac_bool test_msr_mtrrcap_fields();

ac_bool test_msr_perf_power();

#endif
