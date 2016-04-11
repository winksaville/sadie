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

#include <ioapic_x86_print.h>
#include <ioapic_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

/**
 * Print ioapic_redir
 *
 * @param lstr leading string to print
 * @param redir is ioapci_redir to print
 * @param tstr terminating string to print
 */ 
void ioapic_redir_print(char* lstr, ioapic_redir redir, char* tstr) {
  if (lstr == AC_NULL) lstr = "";
  if (tstr == AC_NULL) tstr = "";

  ac_printf("%siv=0x%x dlm=%d dm=%d ds=%d, ip=%d ri=%d t=%d im=%d resv=0x%p df=0x%x%s",
      lstr, redir.intr_vec, redir.delivery_mode, redir.dest_mode,
      redir.delivery_status, redir.intr_polarity, redir.remote_irr,
      redir.trigger, redir.intr_mask, redir.resv, redir.dest_field, tstr);
}
