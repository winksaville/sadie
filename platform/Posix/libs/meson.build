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

# Posix

# Add our platform specific libs
subdir('ac_acpi_impl')
subdir('ac_memmgr')
subdir('ac_pci_impl')
subdir('ac_putchar')
subdir('ac_receptor_impl')
subdir('ac_stop')
subdir('ac_sysconf')
subdir('ac_thread')
subdir('ac_timer')
subdir('ac_tsc_impl')

libruntime = static_library('runtime', runtimeSrcs,
  c_args : compilerArgs,
  include_directories : runtimeIncDirs)

libruntime_dep += declare_dependency(
  link_with : libruntime)
