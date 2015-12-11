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

# ARM Versatile Platform Board
#  Note this isn't a real VersatilePB because a real VersatilePB
#  has an armv5 (arm926) Cpu but I'm using __atomic_xxxx builtins
#  and they aren't available in armv5 so this is a fictional device
#  but allows me to use qemu.

# The name of a subdirectory under arch so arch/armv6zk
Arch = 'armv6'

# The name of a subdirecotry under arch/$Arch so arch/armv6/arm1176jzf-s
Cpu = 'arm1176jzf-s'

# Can be 'hard' or 'soft;
Float = 'hard'

# Load the architecture specific information
subdir('@0@/arch'.format(meson.source_root()))

# And then the libraries for VersatilePB
subdir('libs')
