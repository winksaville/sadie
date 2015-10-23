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

incs = include_directories('incs')

libputchar_dbg = static_library('putchar_dbg', 'srcs/putchar_dbg.c',
  c_args : compilerArgs,
  include_directories : [incs, archIncDirs])

libputchar_dbg_dep = declare_dependency(
  include_directories : [incs],
  link_with : libputchar_dbg)
