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

if Platform == 'Posix'
  srcFiles = firstSrcFiles + ['srcs/test.c']

  # Create test_versatilepb_ac_inet_link_impl executable
  test_versatilepb_ac_inet_link_impl = executable( 'test_versatilepb_ac_inet_link_impl', srcFiles,
    include_directories : runtimeIncDirs,
    link_args : linkArgs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-test-versatilepb-ac_inet_link_impl', test_versatilepb_ac_inet_link_impl)
endif
