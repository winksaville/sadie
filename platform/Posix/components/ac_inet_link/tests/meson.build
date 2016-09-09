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

  # Create test_posix_ac_inet_link_regular executable
  test_posix_ac_inet_link_regular = executable( 'test_posix_ac_inet_link_regular', srcFiles,
    include_directories : incDirs,
    link_args : linkArgs,
    c_args : compilerArgs,
    dependencies : deps,
  )

  # Change t to [1-5] when testing various meson versions
  # For now since 0.35.0 is not released I'm going to use t = 4
  # and use meson version 0.28.0
  t = 4
  if t == 1
    # ERR for: meson version 0.28.0 (no keyword args)
    # ERR for: meson version 0.34.0 (meson fails with: 'CustomTarget' object has no attribute 'replace')
    # OK for: meson version 0.35.0-dev1
    make_root_suid_file =  '@0@/tools/make_root_suid.sh'.format(meson.source_root())
    make_root_suid = find_program(make_root_suid_file)

    test_posix_ac_inet_link = custom_target('test_posix_ac_inet_link',
      input : [test_posix_ac_inet_link_regular],
      output : ['test_posix_ac_inet_link'],
      command : [make_root_suid, '@INPUT@', '@OUTPUT@'],
      depends : [test_posix_ac_inet_link_regular])

    run_target('run-test-posix-ac_inet_link',
      command : [test_posix_ac_inet_link])
  else
    sudo_runner_file =  '@0@/tools/sudo.runner.sh'.format(meson.source_root())
    sudo_runner = find_program(sudo_runner_file)
    if t == 2
      # ERR for: meson version 0.28.0 (no keyword args)
      # ERR for: meson version 0.34.0 (dependencies not built)
      # OK for: meson version 0.35.0-dev1
      run_target('run-test-posix-ac_inet_link_regular',
        command : [sudo_runner, test_posix_ac_inet_link_regular])
    elif t == 3
      # ERR for: meson version 0.28.0 (no keyword args)
      # OK for: meson version 0.34.0
      # OK for: meson version 0.35.0-dev1
      run_target('run-test-posix-ac_inet_link_regular',
        command : [sudo_runner, test_posix_ac_inet_link_regular],
        depends : test_posix_ac_inet_link_regular)
    elif t == 4
      # OK for: meson version 0.28.0
      # ERR for: meson version 0.34.0 (dependencies not built)
      # OK for: meson version 0.35.0-dev1
      run_target('run-test-posix-ac_inet_link_regular', sudo_runner_file, test_posix_ac_inet_link_regular)
    elif t == 5
      # ERR for: meson version 0.28.0 (messon fails with: Invalid argument to run_target)
      # ERR for: meson version 0.34.0 (dependencies not built)
      # OK for: meson version 0.35.0-dev1
      run_target('run-test-posix-ac_inet_link_regular', sudo_runner, test_posix_ac_inet_link_regular)
    endif
  endif
endif
