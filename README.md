# sadie [![Circle CI](https://circleci.com/gh/winksaville/sadie.svg?style=svg)](https://circleci.com/gh/winksaville/sadie)

An experimental runtime based on asychronous components

Currently it run on Posix and qemu-system-arm with -M versatilepb.
And it only outputs 'Hi' to the UART and then shuts down.

License
---
All files unless otherwise indicated are licensed under the
[Apache 2.0 license](http://www.apache.org/licenses/).
That and any other licenses are available under the LICENSES directory.

Prerequesites
---
* [Meson](https://mesonbuild.com) for building, Tested with 0.27
* [binutils-gdb](https://www.gnu.org/software/binutils/) Tested with ToT from git repo
* [Gcc](https://gcc.gnu.org/) Tested with 5.2
* [qemu](http://wiki.qemu.org/Main_Page) Tested with 2.4.0.1

Get the sources for this project from github.
NOTE: this git repo uses submodules so use `git clone --recursive`
```
mkdir -p ~/prgs
cd ~/prgs
git clone --recursive https://github.com/winksaville/sadie.git
cd sadie
```
Create cross-tool-chain
---
```
vendor-install-tools/install.py all
```
Build sadie for ARM VersatilePB
---
```
mkdir -p ~/prgs/sadie/build-VersatilePB
cd ~/prgs/sadie/build-VersatilePB
meson -D Platform=VersatilePB --cross-file ../cross-file-VersatilePB --buildtype plain ..
ninja
```
Build sadie for Posix
---
```
mkdir -p ~/prgs/sadie/build-Posix
cd ~/prgs/sadie/build-Posix
meson -D Platform=Posix ..
ninja
```
Run either of them, cd to the directory and:
---
```
ninja run-test-putchar_dbg
```
Testing
---
[CircleCi](https://circleci.com/home) is used for [testing sadie](https://circleci.com/gh/winksaville/sadie)
