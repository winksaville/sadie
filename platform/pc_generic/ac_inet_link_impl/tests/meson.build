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

incDirs = runtimeIncDirs

#incDirs += include_directories('incs')

if Platform == 'pc_x86_32'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_pc_x86_32_ac_inet_link_impl executable
  test_pc_x86_32_ac_inet_link_impl = executable( 'test_pc_x86_32_ac_inet_link_impl', srcFiles,
    include_directories : incDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-pc_x86_32_ac_inet_link_impl', '@0@/tools/qemu-system-i386.runner.sh'.format(meson.source_root()),
             test_pc_x86_32_ac_inet_link_impl)
endif


if Platform == 'pc_x86_64'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-n,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_pc_x86_64_ac_inet_link_impl executable
  test_pc_x86_64_ac_inet_link_impl = executable( 'test_pc_x86_64_ac_inet_link_impl', srcFiles,
    include_directories : incDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  grub_cfg = '@0@/grub.cfg'.format(meson.current_source_dir())
  test_pc_x86_64_ac_inet_link_impl_exe = '@0@/test_pc_x86_64_ac_inet_link_impl'.format(meson.current_build_dir())

  # Create test_pc_x86_64_ac_inet_link_impl.bin suitable for executing with qemu or on hardware
  test_pc_x86_64_ac_inet_link_impl_bin = custom_target( 'test_pc_x86_64_ac_inet_link_impl.img',
    input : grub_cfg,
    output : 'test_pc_x86_64_ac_inet_link_impl.img',
    command : ['@0@/tools/grub-mkrescue.runner.sh'.format(meson.source_root()),
      test_pc_x86_64_ac_inet_link_impl_exe, grub_cfg, '@OUTPUT@'],
    depends : [test_pc_x86_64_ac_inet_link_impl])

  run_target('run-test-pc_x86_64_ac_inet_link_impl', '@0@/tools/qemu-system-x86_64.runner.sh'.format(meson.source_root()),
              test_pc_x86_64_ac_inet_link_impl_bin, '-enable-kvm', '-cpu', 'host,+tsc-deadline')
endif

