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

project('sadie', ['c'])

pkgconfig_gen(
  version : '0.0.1',
  name : 'sadie',
  filebase : 'sadie',
  description : 'Runtime for asynchronous components')

# Initialize some globals
compilerArgs = ['-std=c11', '-O2', '-g']
linkArgs = []
linkDeps = []
srcFiles = []
runtimeSrcs = []
runtimeIncDirs = []

# Get options
Platform = get_option('Platform')

if Platform == 'VersatilePB'
  subdir('platform/VersatilePB')
endif

if Platform == 'Posix'
  subdir('platform/Posix')
endif

if Platform == 'pc_i586'
  subdir('platform/pc_i586')
endif

# tests
subdir('tests')
