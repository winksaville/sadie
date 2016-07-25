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

# The name of a subdirectory under arch so arch/x86
Arch = 'x86'

# The name of a subdirectory under arch/x86 so arch/x86/x86_64
Cpu = 'x86_64'

# The compiler arguments
compilerArgs += [
  '-D@0@=1'.format(Platform),
  '-D@0@=1'.format(Arch),
  '-DAC_PLATFORM=@0@'.format(Platform),
  '-DAC_ARCH=@0@'.format(Arch),
  '-static',
  '-ffreestanding',
  '-nostartfiles',
  '-nodefaultlibs',
  '-nostdlib',
]
