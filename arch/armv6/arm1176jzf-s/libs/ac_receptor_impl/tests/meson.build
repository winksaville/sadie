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
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test-ac_receptor_impl executable
  test_ac_receptor_impl = executable( 'test_ac_receptor_impl', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create test.bin suitable for executing with qemu
  test_ac_receptor_impl_bin = custom_target( 'test_ac_receptor_impl_bin',
    output : ['test_ac_receptor_impl.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/test_ac_receptor_impl'.format(meson.current_build_dir()),
      '@0@/test_ac_receptor_impl.bin'.format(meson.current_build_dir())],
    depends : [test_ac_receptor_impl])

  run_target('run-test-ac_receptor_impl',
      '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
      'versatilepb', test_ac_receptor_impl_bin)
endif
