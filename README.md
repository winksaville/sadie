# sadie [![Circle CI](https://circleci.com/gh/winksaville/sadie.svg?style=svg)](https://circleci.com/gh/winksaville/sadie)

An experimental runtime based on asychronous components

Currently it runs on Posix and qemu-system-arm with -M versatilepb
and only outputs 'Hi' to the "UART" and then exits.

License
---
All files, unless otherwise indicated, are licensed under the
[Apache 2.0 license](http://www.apache.org/licenses/).
That and any other licenses are available under the LICENSES directory.

Prerequesites
---
* [Meson](https://mesonbuild.com) for building, Tested with 0.27
* [binutils-gdb](https://www.gnu.org/software/binutils/) Tested with ToT from git repo
* [Gcc](https://gcc.gnu.org/) Tested with 5.2
* [qemu](http://wiki.qemu.org/Main_Page) Tested with 2.4.0.1

Sources
---
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
ninja run-test-ac_putchar
```
Testing
---
[CircleCi](https://circleci.com/home) is used for [testing sadie](https://circleci.com/gh/winksaville/sadie)

Notes
---
For some x86_64 applications like test-ac_putchar you can test
using qemu by doing:

   ninja run-test-ac_putchar

And the image `test_ac_putchar.img` can also be used on real hardware.
The way I test the image on real hardware is to write the image to a
usb stick using dd and then inserting the usb stick into the test PC
which is configured to boot from a usb stick. The dd command I use
is below, note the `sync` command to besure everything is written:

*WARNING* Using dd can wipe out you HD *WARNING*

  sudo dd bs=4M if=tests/test-ac_putchar/test_ac_putchar.img of=/dev/sdb ; sync
