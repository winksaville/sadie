# Copyright 2016 wink saville
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
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test-ac_string executable
  test_ac_swap_bytes = executable( 'test_ac_swap_bytes', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create test.bin suitable for executing with qemu
  test_ac_swap_bytes_bin = custom_target( 'test_ac_swap_bytes_bin',
    output : ['test_ac_swap_bytes.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/test_ac_swap_bytes'.format(meson.current_build_dir()),
      '@0@/test_ac_swap_bytes.bin'.format(meson.current_build_dir())],
    depends : [test_ac_swap_bytes])

  run_target('run-test-ac_swap_bytes', '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
              'versatilepb', test_ac_swap_bytes_bin)
endif


if Platform == 'Posix'
  srcFiles = firstSrcFiles + ['srcs/test.c']

  # Create testit executable
  test_ac_swap_bytes = executable( 'test_ac_swap_bytes', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_swap_bytes', test_ac_swap_bytes)
endif

if Platform == 'pc_x86_32'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_swap_bytes executable
  test_ac_swap_bytes = executable( 'test_ac_swap_bytes', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_swap_bytes', '@0@/tools/qemu-system-i386.runner.sh'.format(meson.source_root()),
             test_ac_swap_bytes)
endif


if Platform == 'pc_x86_64'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-n,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_swap_bytes executable
  test_ac_swap_bytes = executable( 'test_ac_swap_bytes', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  grub_cfg = '@0@/grub.cfg'.format(meson.current_source_dir())
  test_ac_swap_bytes_exe = '@0@/test_ac_swap_bytes'.format(meson.current_build_dir())

  # Create test_ac_swap_bytes.bin suitable for executing with qemu or on hardware
  test_ac_swap_bytes_bin = custom_target( 'test_ac_swap_bytes.img',
    input : grub_cfg,
    output : 'test_ac_swap_bytes.img',
    command : ['@0@/tools/grub-mkrescue.runner.sh'.format(meson.source_root()),
      test_ac_swap_bytes_exe, grub_cfg, '@OUTPUT@'],
    depends : [test_ac_swap_bytes])

  run_target('run-test-ac_swap_bytes', '@0@/tools/qemu-system-x86_64.runner.sh'.format(meson.source_root()),
              test_ac_swap_bytes_bin, '-enable-kvm', '-cpu', 'host,+tsc-deadline')
endif

