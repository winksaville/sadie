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

if Platform == 'pc_x86_32'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_apic_x86 executable
  test_apic_x86 = executable( 'test_apic_x86', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-apic_x86', '@0@/tools/qemu-system-i386.runner.sh'.format(meson.source_root()),
             test_apic_x86)
endif


if Platform == 'pc_x86_64'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-n,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test_apic_x86 executable
  test_apic_x86 = executable( 'test_apic_x86', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  grub_cfg = '@0@/grub.cfg'.format(meson.current_source_dir())
  test_apic_x86_exe = '@0@/test_apic_x86'.format(meson.current_build_dir())

  # Create test_apic_x86.bin suitable for executing with qemu or on hardware
  test_apic_x86_bin = custom_target( 'test_apic_x86.img',
    input : grub_cfg,
    output : 'test_apic_x86.img',
    command : ['@0@/tools/grub-mkrescue.runner.sh'.format(meson.source_root()),
      test_apic_x86_exe, grub_cfg, '@OUTPUT@'],
    depends : [test_apic_x86])

  run_target('run-test-apic_x86', '@0@/tools/qemu-system-x86_64.runner.sh'.format(meson.source_root()),
              test_apic_x86_bin, '-enable-kvm', '-cpu', 'host,+tsc-deadline')
endif

