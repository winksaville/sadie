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

#include <page_table_x86.h>

#include <cr_x86.h>
#include <msr_x86.h>

#include <ac_inttypes.h>

/**
 * Return the current cpu page_mode
 */
enum page_mode get_page_mode(void) {
  enum page_mode pm;

  union cr0_u cr0u = { .raw = get_cr0() };
  union cr4_u cr4u = { .raw = get_cr4() };
  union msr_efer_u eferu = { .raw = get_msr(MSR_EFER) };

  if (cr0u.fields.pg == 1) {
    if ((cr4u.fields.pae == 0) && (eferu.fields.lme == 0)) {
      pm = PAGE_MODE_NRML_32BIT;
    } else if ((cr4u.fields.pae == 1) && (eferu.fields.lme == 0)) {
      pm = PAGE_MODE_PAE_32BIT;
    } else if ((cr4u.fields.pae == 1) && (eferu.fields.lme == 1)) {
      if (cr4u.fields.pcide == 0) {
        pm = PAGE_MODE_NRML_64BIT;
      } else {
        pm = PAGE_MODE_PCIDE_64BIT;
      }
    } else {
      pm = PAGE_MODE_UNKNOWN;
    }
  } else {
    pm = PAGE_MODE_UNKNOWN;
  }

  return pm;
}
