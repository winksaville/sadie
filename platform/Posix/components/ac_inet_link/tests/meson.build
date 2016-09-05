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
deps = [ component_dep, libruntime_dep ]

if Platform == 'Posix'
  srcFiles = firstSrcFiles + ['srcs/test.c']

  # Create test_posix_ac_inet_link executable
  test_posix_ac_inet_link = executable( 'test_posix_ac_inet_link', srcFiles,
    include_directories : incDirs,
    link_args : linkArgs,
    c_args : compilerArgs,
    dependencies : deps,
  )

  if false
    in_file = '@0@/test_posix_ac_inet_link'.format(meson.current_build_dir())
    out_file = '@0@/test_posix_ac_inet_link_x'.format(meson.current_build_dir())
    make_root_suid_file =  '@0@/tools/make_root_suid.sh'.format(meson.source_root())

    make_root_suid = find_program(make_root_suid_file)

    test_posix_ac_inet_link_x = custom_target('test_posix_ac_inet_link_x',
      input : [in_file],
      output : ['test_posix_ac_inet_link_x'],
      command : [make_root_suid, '@INPUT@', '@OUTPUT@'],
      depends : [test_posix_ac_inet_link])

    run_target('run-test-posix-ac_inet_link_x', test_posix_ac_inet_link_x)
  else
    run_target('run-test-posix-ac_inet_link', '@0@/tools/sudo.runner.sh'.format(meson.source_root()),
      test_posix_ac_inet_link)
  endif
endif
