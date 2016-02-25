/*
 * copyright 2015 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#include <cr_x86.h>
#include <multiboot2.h>
#include <page_table_x86.h>
#include <page_table_x86_print.h>
#include <reset_x86.h>

#include <ac_inttypes.h>
#include <ac_memset.h>
#include <ac_printf.h>
#include <ac_sort.h>

void print_multiboot2_tag(struct multiboot2_header_tag* tag) {
  ac_printf("type=%d size=%d\n", tag->type, tag->size);
}

ac_sint compare_mmap_entires(const void* entries, ac_uint idx1, ac_uint idx2) {
  const struct multiboot2_mmap* mmap = (const struct multiboot2_mmap*)entries;

  if (mmap[idx1].base_addr < mmap[idx2].base_addr) {
    return -1;
  } else if (mmap[idx1].base_addr > mmap[idx2].base_addr) {
    return 1;
  } else {
    return 0;
  }
}

void swap_mmap_entires(void* entries, const ac_uint idx1, const ac_uint idx2) {
  struct multiboot2_mmap* mmap = (struct multiboot2_mmap*)entries;
  struct multiboot2_mmap tmp;

  tmp = mmap[idx1];
  mmap[idx1] = mmap[idx2];
  mmap[idx2] = tmp;
}

void ac_init(ac_uptr ptr, ac_uint word) {
  // Check if we have a multiboot signature
  ac_printf("ac_init addr=%p\n", ac_init);
  if (word == 0x36d76289) {
    /** Mulitboot header **/
    ac_u32 total_size = *(ac_u32*)ptr;
    ac_u32 reserved = *(ac_u32*)(ptr + 4);

    struct multiboot2_header_tag* mb_end =
      (struct multiboot2_header_tag*)(ptr + total_size);

    struct multiboot2_header_tag* tag =
      (struct multiboot2_header_tag*)(ptr + 8);

    ac_printf("mb_info: 0x%p\n", ptr);
    ac_printf(" total_size=%d\n", total_size);
    ac_printf(" reserved=0x%x\n", reserved);
    ac_printf(" mb_end=0x%p\n", mb_end);

    while ((tag != AC_NULL) && (tag < mb_end) && (tag->type != 0)
        && (tag->size != 8)) {
      print_multiboot2_tag(tag);

      if (tag->type == 1) {
        char* cmdline = (char*)tag + 8;
        ac_printf(" cmd_line=%s\n", cmdline);
      }

      if (tag->type == 4) {
        struct multiboot2_basic_memory_tag* bm =
          (struct multiboot2_basic_memory_tag*)tag;
        ac_printf(" basic memory info:\n");
        ac_printf("  mem_lower=%dk\n", bm->mem_lower);
        ac_printf("  mem_upper=%dk\n", bm->mem_upper);
      }
      if (tag->type == 6) {
        struct multiboot2_memory_map_tag* mm =
          (struct multiboot2_memory_map_tag*)tag;
        ac_printf(" memory map:\n");
        ac_printf("  entry_size=%d\n", mm->entry_size);
        ac_printf("  entry_version=%d\n", mm->entry_version);
        if (mm->entry_version != 0) {
          ac_printf("ABORTING: file ac_init; Unknown multiboot2 entry_version"
             " %d, expecting 0\n", mm->entry_version);
          reset_x86();
        }
        ac_uint count = (mm->header.size - sizeof(mm->header)) / mm->entry_size;
        ac_sort_by_idx(mm->entries, count, compare_mmap_entires,
            swap_mmap_entires);
        ac_uint total_length = 0;
        for (ac_uint i = 0; i < count; i++) {
          if (mm->entries[i].type == 1) {
            total_length += mm->entries[i].length;
          }
          ac_printf("  %d: base_addr=0x%p length=0x%x type=%d reserved=%x\n", i,
              mm->entries[i].base_addr, mm->entries[i].length,
              mm->entries[i].type, mm->entries[i].reserved);
        }
        ac_printf("total_length=%d(0x%x)\n", total_length, total_length);
        init_page_tables(mm, count);
      }
      tag = multiboot2_next_tag(tag);
    }

  } else {
    /** reset */
    ac_printf("ABORTING: file ac_init; Unknown parameter"
       " ptr=%p, word=0x%x\n", ptr, word);
    reset_x86();
  }

}
