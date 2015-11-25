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

if Platform == 'VersatilePB'
  srcFiles += ['srcs/test.c']
  linkfile = 'meson.link.ld'
  linkArgs += ['-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), linkfile)]
  linkDeps += [linkfile]

  incDirs += runtimeIncDirs

  # Create test_ac_putchar executable
  test_ac_putchar = executable( 'test_ac_putchar', srcFiles,
    include_directories : incDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create test_ac_putchar.bin suitable for executing with qemu
  test_ac_putchar_bin = custom_target( 'test_ac_putchar_bin',
    output : ['test_ac_putchar.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/test_ac_putchar'.format(meson.current_build_dir()),
      '@0@/test_ac_putchar.bin'.format(meson.current_build_dir())],
    depends : [test_ac_putchar])

  run_target('run-test-ac_putchar',
    'qemu-system-arm', '-M', 'versatilepb',
    '-m', '128M', '-nographic', '-no-reboot', '-kernel', test_ac_putchar_bin)
endif


if Platform == 'Posix'
  srcFiles += ['srcs/test.c']

  incDirs += runtimeIncDirs

  # Create test_ac_putchar executable
  test_ac_putchar = executable( 'test_ac_putchar', srcFiles,
    include_directories : incDirs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_putchar', test_ac_putchar)
endif
