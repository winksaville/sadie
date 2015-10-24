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

project('xx1-kernel', ['c'])

pkgconfig_gen(
  version : '0.0.1',
  name : 'xx1-kernel',
  filebase : 'xx1-kernel',
  description : 'Experimental Kernel')

# Initialize some globals
compilerArgs = ['-std=c11', '-O2']
linkArgs = []
linkDeps = []
srcFiles = []
incDirs = []

# Get options
Platform = get_option('Platform')

if Platform == 'VersatilePB'
  subdir('platform/VersatilePB')
endif

# Platform set various flags and must be processed first
subdir('libs')

srcFiles += ['test.c']
linkfile = 'meson.link.ld'
linkArgs += ['-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), linkfile)]
linkDeps += [linkfile]

# Create testit executable
testit = executable( 'testit', srcFiles,
  include_directories : incDirs,
  c_args : compilerArgs,
  link_args : linkArgs,
  link_depends : linkDeps,
  dependencies : [libpoweroff_dep, libputchar_dbg_dep],
)

# Create test.bin suitable for executing with qemu
testbin = custom_target( 'testbin',
  output : ['test.bin'],
  command : ['arm-eabi-objcopy', '-O', 'binary', 'testit', 'test.bin'],
  depends : [testit])

run_target('run', 'qemu-system-arm', '-M', 'versatilepb', '-m', '128M', '-nographic', '-no-reboot', '-kernel', testbin)
