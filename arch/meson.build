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

# Add this as and include directory to easily reference the generic
runtimeIncDirs += [include_directories('@0@'.format(meson.current_source_dir()))]

# Include the directory defined by the Cpu variable set
# in the current platform's meson.build.
subdir(Arch)
