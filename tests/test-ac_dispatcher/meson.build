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
  linkArgs += ['-Wl,-lgcc,-T,@0@/@1@'.format(meson.current_source_dir(), linkfile)]
  linkDeps += [linkfile]

  # Create test-ac_dispatcher executable
  test_ac_dispatcher = executable( 'test_ac_dispatcher', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create test.bin suitable for executing with qemu
  test_ac_dispatcher_bin = custom_target( 'test_ac_dispatcher_bin',
    output : ['test_ac_dispatcher.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/test_ac_dispatcher'.format(meson.current_build_dir()),
      '@0@/test_ac_dispatcher.bin'.format(meson.current_build_dir())],
    depends : [test_ac_dispatcher])

  run_target('run-test-ac_dispatcher',
     '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
     'versatilepb', test_ac_dispatcher_bin)
endif


if Platform == 'Posix'
  srcFiles += ['srcs/test.c']

  # Create testit executable
  test_ac_dispatcher = executable( 'test_ac_dispatcher', srcFiles,
    include_directories : runtimeIncDirs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-ac_dispatcher', test_ac_dispatcher)
endif
