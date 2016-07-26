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

#ifndef ARCH_X86_MULTIBOOT2_INCS_MULTIBOOT2_H
#define ARCH_X86_MULTIBOOT2_INCS_MULTIBOOT2_H

#include <ac_inttypes.h>

struct AC_ATTR_PACKED multiboot2_header_tag {
  ac_u32 type;
  ac_u32 size;
};

struct AC_ATTR_PACKED multiboot2_basic_memory_tag {
  struct multiboot2_header_tag header;
  ac_u32 mem_lower;
  ac_u32 mem_upper;
};

struct AC_ATTR_PACKED multiboot2_mmap {
  ac_u64 base_addr;
  ac_u64 length;
  ac_u32 type;
  ac_u32 reserved;
};

struct AC_ATTR_PACKED multiboot2_memory_map_tag {
  struct multiboot2_header_tag header;
  ac_u32 entry_size;
  ac_u32 entry_version;
  struct multiboot2_mmap entries[];
};

static __inline__ struct multiboot2_header_tag* multiboot2_next_tag(
    struct multiboot2_header_tag* tag) {
  void* next = (void*)((((ac_uptr)tag + tag->size) + 7) & 0xfffffffffffffff8);
  if (next == tag) {
    next = AC_NULL;
  }

  return (struct multiboot2_header_tag*)next;
}

#endif
