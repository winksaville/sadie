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

#include <print_page_table_x86.h>
//#include <cr_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

/**
 * Print the page table whose top most directory is
 * table and the mode is one of the three modes supported
 * by x86 cpus.
 */
void print_page_table(union cr3_u cr3u, enum page_mode mode) {
  ac_printf("page directory addr=%p mode=%d\n",
      get_page_directory_addr(cr3u), mode);
}
