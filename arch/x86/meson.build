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

runtimeIncDirs += [include_directories('incs')]

compilerArgs += [
 '-mno-sse', '-mno-sse2', '-mno-sse3', '-mno-sse4',
 '-mno-ssse3', '-mno-mmx', '-mno-3dnow', '-mno-avx',
]

runtimeSrcs += [
  '@0@/srcs/descriptors_x86.c'.format(meson.current_source_dir()),
  '@0@/srcs/descriptors_x86_print.c'.format(meson.current_source_dir()),
  '@0@/srcs/interrupts_x86.c'.format(meson.current_source_dir()),
  '@0@/srcs/reset_x86.c'.format(meson.current_source_dir()),
]

# Include the directory pointed to in the Cpu variable. The
# Cpu variable is set in the current platform's meson.build.
subdir(Cpu)
