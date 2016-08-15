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

lclSrcs = ['srcs/perf_ac_mpsc_ring_buff.c' ]
lclIncDirs = [include_directories('../../')]

if Platform == 'VersatilePB'
  srcFiles = firstSrcFiles + lclSrcs
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create perf-ac_string executable
  perf_ac_mpsc_ring_buff = executable( 'perf_ac_mpsc_ring_buff', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  # Create perf.bin suitable for executing with qemu
  perf_ac_mpsc_ring_buff_bin = custom_target( 'perf_ac_mpsc_ring_buff_bin',
    output : ['perf_ac_mpsc_ring_buff.bin'],
    command : ['arm-eabi-objcopy', '-O', 'binary',
      '@0@/perf_ac_mpsc_ring_buff'.format(meson.current_build_dir()),
      '@0@/perf_ac_mpsc_ring_buff.bin'.format(meson.current_build_dir())],
    depends : [perf_ac_mpsc_ring_buff])

  run_target('run-perf-ac_mpsc_ring_buff', '@0@/tools/qemu-system-arm.runner.sh'.format(meson.source_root()),
              'versatilepb', perf_ac_mpsc_ring_buff_bin)
endif


if Platform == 'Posix'
  srcFiles = firstSrcFiles + lclSrcs

  # Create perfit executable
  perf_ac_mpsc_ring_buff = executable( 'perf_ac_mpsc_ring_buff', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    link_args : linkArgs,
    c_args : compilerArgs,
    dependencies : [libruntime_dep],
  )

  run_target('run-perf-ac_mpsc_ring_buff', perf_ac_mpsc_ring_buff)
endif

if Platform == 'pc_x86_32'
  srcFiles = firstSrcFiles + llcSrcs
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create perf_ac_mpsc_ring_buff executable
  perf_ac_mpsc_ring_buff = executable( 'perf_ac_mpsc_ring_buff', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  run_target('run-perf-ac_mpsc_ring_buff', '@0@/tools/qemu-system-i386.runner.sh'.format(meson.source_root()),
             perf_ac_mpsc_ring_buff)
endif


if Platform == 'pc_x86_64'
  srcFiles = firstSrcFiles + lclSrcs
  linkfile = '@0@/platform/@1@/meson.link.ld'.format(meson.source_root(), Platform)
  linkArgs += ['-Wl,-n,-lgcc,-T,@0@'.format(linkfile)]
  linkDeps += [linkfile]

  # Create perf_ac_mpsc_ring_buff executable
  perf_ac_mpsc_ring_buff = executable( 'perf_ac_mpsc_ring_buff', srcFiles,
    include_directories : runtimeIncDirs + lclIncDirs,
    c_args : compilerArgs,
    link_args : linkArgs,
    link_depends : linkDeps,
    dependencies : [libruntime_dep],
  )

  grub_cfg = '@0@/grub.cfg'.format(meson.current_source_dir())
  perf_ac_mpsc_ring_buff_exe = '@0@/perf_ac_mpsc_ring_buff'.format(meson.current_build_dir())

  # Create perf_ac_mpsc_ring_buff.bin suitable for executing with qemu or on hardware
  perf_ac_mpsc_ring_buff_bin = custom_target( 'perf_ac_mpsc_ring_buff.img',
    input : grub_cfg,
    output : 'perf_ac_mpsc_ring_buff.img',
    command : ['@0@/tools/grub-mkrescue.runner.sh'.format(meson.source_root()),
      perf_ac_mpsc_ring_buff_exe, grub_cfg, '@OUTPUT@'],
    depends : [perf_ac_mpsc_ring_buff])

  run_target('run-perf-ac_mpsc_ring_buff', '@0@/tools/qemu-system-x86_64.runner.sh'.format(meson.source_root()),
              perf_ac_mpsc_ring_buff_bin, '-enable-kvm', '-cpu', 'host,+tsc-deadline')
endif

