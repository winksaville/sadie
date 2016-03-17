# Copyright 2015 wink saville
#
# licensed under the apache license, version 2.0 (the "license");
# you may not use this file except in compliance with the license.
# you may obtain a copy of the license at
#
#     http://www.apache.org/licenses/license-2.0
#
# unless required by applicable law or agreed to in writing, software
# distributed under the license is distributed on an "as is" basis,
# without warranties or conditions of any kind, either express or implied.
# see the license for the specific language governing permissions and
# limitations under the license.

if (Platform == 'pc_x86_32') or (Platform == 'pc_x86_64') or (Platform == 'Posix')

  subdir('native_x86')
  subdir('cpuid_x86')

  if (Platform == 'pc_x86_32') or (Platform == 'pc_x86_64')
    subdir('ac_receptor_impl')
    subdir('ac_tsc_impl')
    subdir('apic_x86')
    subdir('cr_x86')
    subdir('descriptors_x86')
    subdir('interrupts_x86')
    subdir('io_x86')
    subdir('msr_x86')
    subdir('reset_x86')
    subdir('page_table_x86')
    subdir('thread_x86')
  endif

endif
