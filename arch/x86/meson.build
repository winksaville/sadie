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

  runtimeIncDirs += [include_directories('incs')]

  runtimeSrcs += [
    '@0@/srcs/reset_x86.c'.format(meson.current_source_dir()),
  ]

  if (Platform != 'Posix')
    # Interrupts not supported on Posix
    runtimeSrcs += [
      '@0@/srcs/interrupts_x86.c'.format(meson.current_source_dir()),
    ]
  endif

  subdir('apic_x86')
  subdir('cpuid_x86')
  subdir('descriptors_x86')
endif

# Include the directory pointed to in the Cpu variable. The
# Cpu variable is set in the current platform's meson.build.
subdir(Cpu)
