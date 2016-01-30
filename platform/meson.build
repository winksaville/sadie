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

# Use Platform variable set in <root> meson.build to decide
# which platform subdir to use.
if Platform == 'VersatilePB'
  subdir('VersatilePB')
elif Platform == 'Posix'
  subdir('Posix')
elif Platform == 'pc_x86_64'
  subdir('pc_x86_64')
elif Platform == 'pc_x86_32'
  subdir('pc_x86_32')
else
  error('Unknown Platform @0@'.format(Platform))
endif

# platform tests
subdir('tests')
