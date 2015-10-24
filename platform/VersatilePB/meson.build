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

# ARM Versatile Platform Board

# The name of the subdirectory directly under arch/
Arch = 'arm_32'

# CFlags that could be useful to other builds,
# such as arc/arm_32/meson.build
CFlagMArch = 'armv5te'
CFlagMCpu = 'arm926ej-s'
CFlagMTune = 'arm926ej-s'
CFlagMFloatAbi = 'soft'

# The compiler arguments
compilerArgs += [
  '-D@0@'.format(Platform),
  '-D@0@'.format(Arch),
  '-march=@0@'.format(CFlagMArch),
  '-mcpu=@0@'.format(CFlagMCpu),
  '-mtune=arm926ej-s',
  '-msoft-float',
  '-mfloat-abi=soft',
]

subdir('@0@/arch/@1@'.format(meson.source_root(), Arch))

incs = include_directories('incs')

libruntime = static_library('runtime', 'srcs/poweroff.c', 'srcs/startup.S',
  c_args : compilerArgs,
  include_directories : [incs, archIncDirs])

libruntime_dep = declare_dependency(
  include_directories : [incs],
  link_with : libruntime)
