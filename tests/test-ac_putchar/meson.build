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
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_putchar executable
  test_ac_putchar = executable( 'test_ac_putchar', srcFiles,
    include_directories : runtimeIncDirs,
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

  run_target('run-test-ac_putchar', '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
              'versatilepb', test_ac_putchar_bin)
endif


if Platform == 'Posix'
  srcFiles += ['srcs/test.c']

  # Create test_ac_putchar executable
  test_ac_putchar = executable( 'test_ac_putchar', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_putchar', test_ac_putchar)
endif

if Platform == 'pc_i586'
  srcFiles += ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_putchar executable
  test_ac_putchar = executable( 'test_ac_putchar', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_putchar', '@0@/tools/qemu-system-i386.runner.sh'.format(meson.source_root()),
             test_ac_putchar)
endif

if Platform == 'pc_x86_64'
  srcFiles += ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-n,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_putchar executable
  test_ac_putchar = executable( 'test_ac_putchar', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create test_ac_putchar.bin suitable for executing with qemu
  test_ac_putchar_bin = custom_target( 'test_ac_putchar.img',
    output : ['test_ac_putchar.img'],
    command : ['@0@/tools/grub-mkrescue.runner.sh'.format(meson.source_root()),
      '@0@/test_ac_putchar'.format(meson.current_build_dir()),
      '@0@/grub.cfg'.format(meson.current_source_dir()),
      '@0@/test_ac_putchar.img'.format(meson.current_build_dir())],
    depends : [test_ac_putchar])

  run_target('run-test-ac_putchar', '@0@/tools/qemu-system-x86_64.runner.sh'.format(meson.source_root()),
              test_ac_putchar_bin)
endif

