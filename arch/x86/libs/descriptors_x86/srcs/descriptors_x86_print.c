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

#include "ac_inttypes.h"
#include "ac_printf.h"
#include "descriptors_x86.h"
#include "descriptors_x86_print.h"

void print_tss_desc(char *str, TssDesc* desc) {
  if (str != AC_NULL) {
    ac_printf("%s: ", str);
  }
  union tss_desc_u tss_u = { .fields = *desc, };
  ac_printf("0x%lx\n", tss_u.raw);

  ac_printf(" desc->seg_limit_lo: 0x%lx\n", desc->seg_limit_lo);
  ac_printf(" desc->base_addr_lo: 0x%lx\n", desc->base_addr_lo);
  ac_printf(" desc->type: %d\n", desc->type);
  ac_printf(" desc->unused_1: %d\n", desc->unused_1);
  ac_printf(" desc->dpl: %d\n", desc->dpl);
  ac_printf(" desc->p: %d\n", desc->p);
  ac_printf(" desc->seg_limit_hi: 0x%lx\n", desc->seg_limit_hi);
  ac_printf(" desc->avl: %d\n", desc->avl);
  ac_printf(" desc->unused_2: %d\n", desc->unused_2);
  ac_printf(" desc->g: %d\n", desc->g);
  ac_printf(" desc->base_addr_hi: 0x%lx\n", desc->base_addr_hi);
#ifdef CPU_X86_64
  ac_printf(" desc->unused_3: %d\n", desc->unused_3);
#endif
  ac_printf(" desc->seg_limit: 0x%lx\n", GET_TSS_DESC_SEG_LIMIT(*desc));
  ac_printf(" desc->base_addr: 0x%lx\n", GET_TSS_DESC_BASE_ADDR(*desc));
}

void print_seg_desc(char *str, SegDesc* desc) {
  if (str != AC_NULL) {
    ac_printf("%s: ", str);
  }
  union seg_desc_u sd_u = { .fields = *desc, };
  ac_printf("0x%lx\n", sd_u.raw);

  ac_printf(" desc->seg_limit_lo: 0x%lx\n", desc->seg_limit_lo);
  ac_printf(" desc->base_addr_lo: 0x%lx\n", desc->base_addr_lo);
  ac_printf(" desc->type: %d\n", desc->type);
  ac_printf(" desc->s: %d\n", desc->s);
  ac_printf(" desc->dpl: %d\n", desc->dpl);
  ac_printf(" desc->p: %d\n", desc->p);
  ac_printf(" desc->seg_limit_hi: 0x%lx\n", desc->seg_limit_hi);
  ac_printf(" desc->avl: %d\n", desc->avl);
  ac_printf(" desc->l: %d\n", desc->l);
  ac_printf(" desc->d_b: %d\n", desc->d_b);
  ac_printf(" desc->g: %d\n", desc->g);
  ac_printf(" desc->base_addr_hi: 0x%lx\n", desc->base_addr_hi);
  ac_printf(" desc->seg_limit: 0x%lx\n", GET_SEG_DESC_SEG_LIMIT(*desc));
  ac_printf(" desc->base_addr: 0x%lx\n", GET_SEG_DESC_BASE_ADDR(*desc));
}

void print_desc_table(char *str, DescPtr dp) {
  ac_uint count = (dp.limit + 1) / sizeof(SegDesc);
  ac_printf("%s: count=%d dp.limit=%d sd=%p\n", str, count, dp.limit, dp.sd);
  for (ac_uint i = 0; i < count; i++) {
    ac_printf("SegDesc[%d]:\n", i);
    print_seg_desc(AC_NULL, &dp.sd[i]);
  }
}
