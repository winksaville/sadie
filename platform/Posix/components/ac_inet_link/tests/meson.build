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

  # Create test_ac_inet_link_impl executable
  test_posix_ac_inet_link = executable( 'test_posix_ac_inet_link', srcFiles,
    include_directories : incDirs,
    link_args : linkArgs,
    c_args : compilerArgs,
    dependencies : deps,
  )

  a = 'f'
  if a == 't'
    #How to have this run command depend on test_posix_ac_inet_link and have it always run?
    # This doesn't work:
    make_suid = find_program('@0@/tools/make.root.suid.sh'.format(meson.source_root()))
    gen = generator(make_suid,
      output : '@PLAINNAME@',
      arguments : 'file_name',
      dependencies : test_posix_ac_inet_link)
    generated = gen.process('test_posix_ac_inet_link')
    run_target('run-test-posix-ac_inet_link', generated)
  else
    run_target('run-test-posix-ac_inet_link', '@0@/tools/sudo.runner.sh'.format(meson.source_root()),
      test_posix_ac_inet_link)
  endif
endif
