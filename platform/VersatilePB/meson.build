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

# The name of the subdirectory directly under arch/
Arch = 'arm_32'

# From (http://www.gurucoding.com/en/rpi_cross_compiler/diff_hardfp_softfp.php)
# the Raspberry Pi main chip is Broadcom BCM2835, and it's a SoC (System-on-Chip).
# This SoC includes CPU, GPU and RAM. CPU is ARM1176JZF-S 700 MHz processor,
# capable of doing floating point math in hardware, including VFP extensions
# (Vector Floating Point), somewhat similar to x86 SIMD called SSE2.
#
# CFlags that could be useful to other builds,
# such as arc/arm_32/meson.build

# Valid arguments to '-march=' are:
#  armv2 armv2a armv3 armv3m armv4 armv4t armv5 armv5e armv5t armv5te
#  armv6 armv6-m armv6j armv6k armv6s-m armv6t2 armv6z armv6zk
#  armv7 armv7-a armv7-m armv7-r armv7e-m armv7ve
#  armv8-a armv8-a+crc
#  iwmmxt iwmmxt2 native

# Original VersatilePB flags
#CFlagMArch = 'armv5te'
#CFlagMCpu = 'arm926ej-s'
#CFlagMTune ='arm926ej-s'
#CFlagMFloatHardSoft = 'soft'

# BCM2635 for RaspberryPi
CFlagMArch = 'armv6zk'
CFlagMCpu = 'arm1176jzf-s'
CFlagMTune = 'arm1176jzf-s'
CFlagMFloatHardSoft = 'hard'

# The compiler arguments
compilerArgs += [
  '-D@0@'.format(Platform),
  '-D@0@'.format(Arch),
  '-march=@0@'.format(CFlagMArch),
  '-mcpu=@0@'.format(CFlagMCpu),
  '-mtune=@0@'.format(CFlagMTune),
  '-m@0@-float'.format(CFlagMFloatHardSoft),
  '-mfloat-abi=@0@'.format(CFlagMFloatHardSoft),
]

subdir('@0@/arch/@1@'.format(meson.source_root(), Arch))

subdir('libs')
