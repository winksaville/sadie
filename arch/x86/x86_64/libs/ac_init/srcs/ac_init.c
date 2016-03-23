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

#include <apic_x86.h>
#include <cr_x86.h>
#include <interrupts_x86.h>
#include <msr_x86.h>
#include <native_x86.h>
#include <page_table_x86.h>
#include <page_table_x86_print.h>
#include <reset_x86.h>

#include <multiboot2.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_memset.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_sort.h>
#include <ac_string.h>
#include <ac_thread.h>
#include <ac_tsc.h>

static void print_multiboot2_tag(struct multiboot2_header_tag* tag) {
  ac_printf("type=%d size=%d\n", tag->type, tag->size);
}

static ac_sint compare_mmap_entires(const void* entries, ac_uint idx1, ac_uint idx2) {
  const struct multiboot2_mmap* mmap = (const struct multiboot2_mmap*)entries;

  if (mmap[idx1].base_addr < mmap[idx2].base_addr) {
    return -1;
  } else if (mmap[idx1].base_addr > mmap[idx2].base_addr) {
    return 1;
  } else {
    return 0;
  }
}

static void swap_mmap_entires(void* entries, const ac_uint idx1, const ac_uint idx2) {
  struct multiboot2_mmap* mmap = (struct multiboot2_mmap*)entries;
  struct multiboot2_mmap tmp;

  tmp = mmap[idx1];
  mmap[idx1] = mmap[idx2];
  mmap[idx2] = tmp;
}

static void initial_page_table(ac_uptr ptr, ac_uint word) {
  // BIOS Data Area (BDA) is at 0x400 and at 0x40E
  // maybe the Extended BIOS Data Area (EBDA)
  ac_u16* bda = (ac_u16*)0x0400;
  for (ac_uint i = 0; i < 9; i++) {
    ac_printf("bda[%d]=0x%x\n", i, bda[i]);
  }
  ac_u64 ebda = bda[7] << 4;
  ac_printf("EBDA=0x%x\n", ebda);

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

      switch (tag->type) {
        case 1: {
          char* cmdline = (char*)tag + 8;
          ac_printf(" cmd_line=%s\n", cmdline);
          break;
        }
        case 2: {
          char* blname = (char*)tag + 8;
          ac_printf(" boot loader name=%s\n", blname);
          break;
        }
        case 3: {
          ac_printf(" Modules\n");
          break;
        }
        case 4: {
          struct multiboot2_basic_memory_tag* bm =
            (struct multiboot2_basic_memory_tag*)tag;
          ac_printf(" basic memory info:\n");
          ac_printf("  mem_lower=%dK(0x%x)\n", bm->mem_lower, bm->mem_lower * 1024);
          ac_printf("  mem_upper=%dK(0x%x)\n", bm->mem_upper, bm->mem_upper * 1024);
          break;
        }
        case 5: {
          ac_printf(" bios boot device\n");
          break;
        }
        case 6: {
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

          // Sort the enteries and create the initial a page_table
          ac_uint count = (mm->header.size - sizeof(mm->header)) / mm->entry_size;
          ac_sort_by_idx(mm->entries, count, compare_mmap_entires,
              swap_mmap_entires);

          init_page_tables(mm, count);

          // Map the possible ebda page so we can search it, I'm
          // assuming its part of a reserved type 2 memory and
          // wasn't mapped above
          ac_u64 ebda_page_addr = ebda & ~AC_BIT_MASK(ac_u64, 12);
          ac_printf("ebda_page_addr=0x%p\n", ebda_page_addr);
          page_table_map_lin_to_phy(get_page_table_linear_addr(),
              (void*)ebda_page_addr, ebda_page_addr, FOUR_K_PAGE_SIZE,
              PAGE_CACHING_WRITE_BACK);

          // Display the types and while doing so search for
          // the MP Floaing Point structure from the Intel
          // MultiProcessor Specification v1.4.
          ac_uint total_length = 0;
          for (ac_uint i = 0; i < count; i++) {
            if (mm->entries[i].type == 1) {
              total_length += mm->entries[i].length;
            }
            ac_printf("  %d: base_addr=0x%p length=0x%x type=%d reserved=%x\n", i,
                mm->entries[i].base_addr, mm->entries[i].length,
                mm->entries[i].type, mm->entries[i].reserved);
            // Search for _MP_ in EBDA
            if ((mm->entries[i].type == 1) ||
                ((mm->entries[i].type == 2)
                  && (mm->entries[i].base_addr == ebda))) {
              // RAM or Probable ebda
              ac_printf("  Look for MP Floating Point Structure\n");
              ac_u8* p = (ac_u8*)mm->entries[i].base_addr;
              //char* nl = "";
              for(ac_uint j = 0; j < 1024; j++) {
                //if ((j % 8) == 0) ac_printf("%s0x%p:", nl, &p[j]);
                //nl = "\n";
                //ac_printf(" %x", p[j]);
                if (ac_strncmp((char*)&p[j], "_MP_", 4) == 0) {
                  ac_printf("\nFound _MP_ at &mp[%d]=0x%p\n", j, &p[j]);
                }
              }
              ac_printf("\n");
            }
          }
          ac_printf("  total_length=%d(0x%x)\n", total_length, total_length);
          break;
        }
        case 7: {
          ac_printf(" VBE info\n");
          break;
        }
        case 8: {
          ac_printf(" Framebuffer info\n");
          break;
        }
        case 9: {
          ac_printf(" Elf symbols\n");
          break;
        }
        case 10: {
          ac_printf(" APM table\n");
          break;
        }
        default: {
          break;
        }
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

void ac_init(ac_uptr ptr, ac_uint word) {
  ac_printf("ac_init: flags=0x%x\n", get_flags());

  // Create initial page table
  initial_page_table(ptr, word);

  // Initialize interrupt descriptor table and apic since
  // they are not done by default, yet.
  initialize_intr_descriptor_table();

  // Initialize receptor module
  ac_receptor_early_init();

  // Initialize Advanced Programmable Interrupt Controller
  if (initialize_apic() != 0) {
    /** reset */
    ac_printf("ABORTING: file ac_init; initialize_apic failed\n");
    reset_x86();
  }

  // Initialize tsc
  ac_tsc_init();

  // Initialize perf and power msrs
  msr_perf_power_x86_init();

  // Initialize threading module
  ac_thread_early_init();

  // Enable interrupts
  sti();

  ac_printf("ac_init:-flags=0x%x\n", get_flags());
}
