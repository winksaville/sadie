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

incDirs = runtimeIncDirs + componentIncDirs
deps = [ libruntime_dep, component_dep ]

if Platform == 'VersatilePB'
  srcFiles = firstSrcFiles + ['srcs/test.c']
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create test-ac_string executable
  test_versatilepb_ac_inet_link = executable( 'test_versatilepb_ac_inet_link', srcFiles,
    include_directories : incDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : deps,
  )

  # Create test.bin suitable for executing with qemu
  test_versatilepb_ac_inet_link_bin = custom_target( 'test_versatilepb_ac_inet_link_bin',
    output : ['test_versatilepb_ac_inet_link.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/test_versatilepb_ac_inet_link'.format(meson.current_build_dir()),
      '@0@/test_versatilepb_ac_inet_link.bin'.format(meson.current_build_dir())],
    depends : [test_versatilepb_ac_inet_link])

  run_target('run-test-versatilepb-ac_inet_link', '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
              'versatilepb', test_versatilepb_ac_inet_link_bin)
endif
