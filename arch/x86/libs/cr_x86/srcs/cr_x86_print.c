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
  union cr0_u reg = { .raw = cr0 };

  ac_printf("%s: 0x%p\n", str, reg.raw);
  ac_printf(" pe=%d\n", reg.fields.pe);
  ac_printf(" mp=%d\n", reg.fields.mp);
  ac_printf(" em=%d\n", reg.fields.em);
  ac_printf(" ts=%d\n", reg.fields.ts);
  ac_printf(" et=%d\n", reg.fields.et);
  ac_printf(" ne=%d\n", reg.fields.ne);
  ac_printf(" wp=%d\n", reg.fields.wp);
  ac_printf(" am=%d\n", reg.fields.pe);
  ac_printf(" nw=%d\n", reg.fields.nw);
  ac_printf(" cd=%d\n", reg.fields.cd);
  ac_printf(" pg=%d\n", reg.fields.pg);
}

void print_cr3(char* str, ac_uint cr3) {
  union cr3_u reg = { .raw = cr3 };

  ac_printf("%s: 0x%x\n", str, reg.raw);
  ac_printf(" val=%x\n", reg.fields.val);
}

void print_cr4(char* str, ac_uint cr4) {
  union cr4_u reg = { .raw = cr4 };

  ac_printf("%s: 0x%p\n", str, reg.raw);
  ac_printf(" vme=%d\n", reg.fields.vme);
  ac_printf(" pvi=%d\n", reg.fields.pvi);
  ac_printf(" tsd=%d\n", reg.fields.tsd);
  ac_printf(" de=%d\n", reg.fields.de);
  ac_printf(" pse=%d\n", reg.fields.pse);
  ac_printf(" pae=%d\n", reg.fields.pae);
  ac_printf(" mce=%d\n", reg.fields.mce);
  ac_printf(" pge=%d\n", reg.fields.pge);
  ac_printf(" pce=%d\n", reg.fields.pce);
  ac_printf(" osfxsr=%d\n", reg.fields.osfxsr);
  ac_printf(" osxmmexcpt=%d\n", reg.fields.osxmmexcpt);
  ac_printf(" fsgsbase=%d\n", reg.fields.fsgsbase);
  ac_printf(" pcide=%d\n", reg.fields.pcide);
  ac_printf(" oxsave=%d\n", reg.fields.oxsave);
  ac_printf(" smep=%d\n", reg.fields.smep);
  ac_printf(" smap=%d\n", reg.fields.smap);
  ac_printf(" pke=%d\n", reg.fields.pke);
}
