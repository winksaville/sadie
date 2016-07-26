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

#include <descriptors_x86.h>

#include <ac_inttypes.h>
#include <ac_memcmp.h>

void set_seg_desc(SegDesc* sd, ac_u32 seg_limit, ac_uptr base_addr, ac_u8 type,
    ac_u8 s, ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 l, ac_u8 d_b, ac_u8 g) {
  SegDesc default_desc = SEG_DESC_INITIALIZER;

  *sd = default_desc;
  sd->seg_limit_lo = SEG_DESC_SEG_LIMIT_LO(seg_limit);
  sd->seg_limit_hi = SEG_DESC_SEG_LIMIT_HI(seg_limit);
  sd->base_addr_lo = SEG_DESC_BASE_ADDR_LO(base_addr);
  sd->base_addr_hi = SEG_DESC_BASE_ADDR_HI(base_addr);
  sd->type = type;
  sd->s = s;
  sd->dpl = dpl;
  sd->p = p;
  sd->avl = avl;
  sd->l = l;
  sd->d_b = d_b;
  sd->g = g;
}

void set_code_seg_desc(SegDesc* sd, ac_u8 accessed, ac_u8 readable, ac_u8 conforming,
    ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 l, ac_u8 d, ac_u8 g) {
    union seg_type_u type = {
      .code = {
        .a = accessed, .r = readable, .c = conforming, .one = 1
      },
  };

  set_seg_desc(sd, 0, 0, type.byte, 1, dpl, p, avl, l, d, g);
}

void set_data_seg_desc(SegDesc* sd, ac_u8 accessed, ac_u8 write_enable, ac_u8 expand_dir,
    ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 l, ac_u8 d, ac_u8 g) {
  union seg_type_u type = {
      .data = {
        .a = accessed, .w = write_enable, .e = expand_dir, .zero = 0
      },
  };

  set_seg_desc(sd, 0, 0, type.byte, 1, dpl, p, avl, l, d, g);
}

ac_s32 cmp_seg_desc(SegDesc* sd1, SegDesc* sd2)  {
  return ac_memcmp(sd1, sd2, sizeof(SegDesc));
}

void set_tss_desc(TssDesc* td, ac_u32 seg_limit, ac_uptr base_addr, ac_u8 type,
    ac_u8 dpl, ac_u8 p, ac_u8 avl, ac_u8 g) {

  TssDesc default_desc = TSS_DESC_INITIALIZER;

  *td = default_desc;
  td->seg_limit_lo = TSS_DESC_SEG_LIMIT_LO(seg_limit);
  td->seg_limit_hi = TSS_DESC_SEG_LIMIT_HI(seg_limit);
  td->base_addr_lo = TSS_DESC_BASE_ADDR_LO(base_addr);
  td->base_addr_hi = TSS_DESC_BASE_ADDR_HI(base_addr);
  td->type = type;
  td->dpl = dpl;
  td->p = p;
  td->avl = avl;
  td->g = g;
}

ac_s32 cmp_tss_desc(TssDesc* sd1, TssDesc* sd2)  {
  return ac_memcmp(sd1, sd2, sizeof(TssDesc));
}
