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

#include <cr_x86_print.h>
#include <cr_x86.h>

#include <ac_printf.h>

void print_cr0(char* str, ac_uint cr0) {
  union cr0_u reg = { .data = cr0 };

  ac_printf("%s: 0x%p\n", str, reg.data);
  ac_printf(" pe=%d\n", reg.bits.pe);
  ac_printf(" mp=%d\n", reg.bits.mp);
  ac_printf(" em=%d\n", reg.bits.em);
  ac_printf(" ts=%d\n", reg.bits.ts);
  ac_printf(" et=%d\n", reg.bits.et);
  ac_printf(" ne=%d\n", reg.bits.ne);
  ac_printf(" wp=%d\n", reg.bits.wp);
  ac_printf(" am=%d\n", reg.bits.pe);
  ac_printf(" nw=%d\n", reg.bits.nw);
  ac_printf(" cd=%d\n", reg.bits.cd);
  ac_printf(" pg=%d\n", reg.bits.pg);
}

void print_cr3(char* str, ac_uint cr3) {
  union cr3_u reg = { .data = cr3 };

  ac_printf("%s: 0x%p\n", str, reg.data);
  ac_printf(" pwt=%d\n", reg.bits.pwt);
  ac_printf(" pcd=%d\n", reg.bits.pcd);
  ac_printf(" page dir addr=0x%p\n", reg.bits.page_directory_base << 12);
}

void print_cr4(char* str, ac_uint cr4) {
  union cr4_u reg = { .data = cr4 };

  ac_printf("%s: 0x%p\n", str, reg.data);
  ac_printf(" vme=%d\n", reg.bits.vme);
  ac_printf(" pvi=%d\n", reg.bits.pvi);
  ac_printf(" tsd=%d\n", reg.bits.tsd);
  ac_printf(" de=%d\n", reg.bits.de);
  ac_printf(" pse=%d\n", reg.bits.pse);
  ac_printf(" pae=%d\n", reg.bits.pae);
  ac_printf(" mce=%d\n", reg.bits.mce);
  ac_printf(" pge=%d\n", reg.bits.pge);
  ac_printf(" pce=%d\n", reg.bits.pce);
  ac_printf(" osfxsr=%d\n", reg.bits.osfxsr);
  ac_printf(" osxmmexcpt=%d\n", reg.bits.osxmmexcpt);
  ac_printf(" fsgsbase=%d\n", reg.bits.fsgsbase);
  ac_printf(" pcide=%d\n", reg.bits.pcide);
  ac_printf(" oxsave=%d\n", reg.bits.oxsave);
  ac_printf(" smep=%d\n", reg.bits.smep);
  ac_printf(" smap=%d\n", reg.bits.smap);
  ac_printf(" pke=%d\n", reg.bits.pke);
}
