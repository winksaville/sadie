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

lclSrcs = ['srcs/test_ac_msg_pool.c', 'srcs/test_ac_msg_pool_multiple_threads.c']
#lclSrcs = ['srcs/test_ac_msg_pool.c']
lclIncDirs = [include_directories('../../')]

if Platform == 'VersatilePB'
  srcFiles = firstSrcFiles + lclSrcs
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test-ac_string executable
  test_ac_msg_pool = executable( 'test_ac_msg_pool', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create test.bin suitable for executing with qemu
  test_ac_msg_pool_bin = custom_target( 'test_ac_msg_pool_bin',
    output : ['test_ac_msg_pool.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/test_ac_msg_pool'.format(meson.current_build_dir()),
      '@0@/test_ac_msg_pool.bin'.format(meson.current_build_dir())],
    depends : [test_ac_msg_pool])

  run_target('run-test-ac_msg_pool', '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
              'versatilepb', test_ac_msg_pool_bin)
endif


if Platform == 'Posix'
  srcFiles = firstSrcFiles + lclSrcs

  # Create testit executable
  test_ac_msg_pool = executable( 'test_ac_msg_pool', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    link_args : linkArgs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_msg_pool', test_ac_msg_pool)
endif

if Platform == 'pc_x86_32'
  srcFiles = firstSrcFiles + llcSrcs
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_msg_pool executable
  test_ac_msg_pool = executable( 'test_ac_msg_pool', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_msg_pool', '@0@/tools/qemu-system-i386.runner.sh'.format(meson.source_root()),
             test_ac_msg_pool)
endif


if Platform == 'pc_x86_64'
  srcFiles = firstSrcFiles + lclSrcs
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-n,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_ac_msg_pool executable
  test_ac_msg_pool = executable( 'test_ac_msg_pool', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  grub_cfg = '@0@/grub.cfg'.format(meson.current_source_dir())
  test_ac_msg_pool_exe = '@0@/test_ac_msg_pool'.format(meson.current_build_dir())

  # Create test_ac_msg_pool.bin suitable for executing with qemu or on hardware
  test_ac_msg_pool_bin = custom_target( 'test_ac_msg_pool.img',
    input : grub_cfg,
    output : 'test_ac_msg_pool.img',
    command : ['@0@/tools/grub-mkrescue.runner.sh'.format(meson.source_root()),
      test_ac_msg_pool_exe, grub_cfg, '@OUTPUT@'],
    depends : [test_ac_msg_pool])

  run_target('run-test-ac_msg_pool', '@0@/tools/qemu-system-x86_64.runner.sh'.format(meson.source_root()),
              test_ac_msg_pool_bin, '-enable-kvm', '-cpu', 'host,+tsc-deadline')
endif

