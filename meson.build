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

project('sadie', ['c'])

pkgg = import('pkgconfig')

pkgg.generate(
  version : '0.0.1',
  name : 'sadie',
  filebase : 'sadie',
  description : 'Runtime for asynchronous components')

# This will be the first files linked and is optional. It is used
# to to add the multi 2 boot file, m2b.S as the first file when
# creating grub bootable files.
firstSrcFiles = []

# Initialize some globals
compilerArgs = ['-std=c11', '-O2', '-g', '-Wall']
linkArgs = []
linkDeps = []
runtimeSrcs = []
runtimeIncDirs = []
libruntime_dep = []

# Get options
Platform = get_option('Platform')

subdir('platform')

# tests

# subdir('arch/test') doesn't work because in arch/tests/meson.build
# I need to use an "absolute path":
#   subdir('@0@/arch/@1@/@2@'.format(meson.source_root(),Arch,'tests'))
# and for some reason this causes the generated code for test-native_x86
# to end up in the source tree so instead I'm invoking the test
# directly from here for now:
#   subdir('arch/x86/tests/test-native_x86')

#subdir('arch/tests')
subdir('arch/x86/libs/ac_tsc_impl/tests')
subdir('arch/x86/libs/ac_keyboard_impl/tests')
subdir('arch/x86/libs/ac_receptor_impl/tests')
subdir('arch/x86/libs/apic_x86/tests')
subdir('arch/x86/libs/cpuid_x86/tests')
subdir('arch/x86/libs/cr_x86/tests')
subdir('arch/x86/libs/descriptors_x86/tests')
subdir('arch/x86/libs/interrupts_x86/tests')
subdir('arch/x86/libs/ioapic_x86/tests')
subdir('arch/x86/libs/msr_x86/tests')
subdir('arch/x86/libs/native_x86/tests')
subdir('arch/x86/libs/page_table_x86/tests')
subdir('arch/x86/libs/thread_x86/tests')

subdir('arch/armv6/arm1176jzf-s/libs/ac_receptor_impl/tests')

subdir('platform/Posix/libs/ac_receptor_impl/tests')
subdir('platform/Posix/libs/ac_tsc_impl/tests')

#subdir('libs/tests')
subdir('libs/ac_acpi/tests')
subdir('libs/ac_bits/tests')
subdir('libs/ac_comp_mgr/tests')
subdir('libs/ac_check_sum/tests')
subdir('libs/ac_mem/tests')
subdir('libs/ac_mpsc_fifo/tests')
subdir('libs/ac_mpscfifo/tests')
subdir('libs/ac_mem_pool/tests')
subdir('libs/ac_msg_pool/tests')
subdir('libs/ac_printf/tests')
subdir('libs/ac_pci/tests')
subdir('libs/ac_time/tests')

subdir('platform/tests')
subdir('tests')
