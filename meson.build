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

# Initialize some globals
compilerArgs = ['-std=c11', '-O2', '-g']
linkArgs = []
linkDeps = []
srcFiles = []
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
subdir('arch/x86/apic_x86/tests')
subdir('arch/x86/cpuid_x86/tests/test-cpuid_x86')
subdir('arch/x86/tests/test-interrupts_x86')
subdir('arch/x86/tests/test-native_x86')
subdir('arch/x86/tests/test-rdwrmsr_x86')

#subdir('libs/tests')
subdir('libs/ac_bits/tests')

subdir('platform/tests')
subdir('tests')
