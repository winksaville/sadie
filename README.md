# xx1-kernel
---
An experimental kernel based on asychronicity.

Currently it only runs on qemu-system-arm with -M versatilepb.
And it only outputs 'Hi' to the UART and then shuts down, so
its not even asynchronous yet.

License
---
All files unless otherwise indicated are licensed under the
[Apache 2.0 license](http://www.apache.org/licenses/).
That and any other licenses are available under the LICENSES directory.

Prerequesites
---
* [Meson](https://mesonbuild.com) for building, Tested with 0.27
* [Gcc](https://gcc.gnu.org/) Tested with 5.2
* [binutils](https://www.gnu.org/software/binutils/) Tested with 2.25
* [qemu](http://wiki.qemu.org/Main_Page) Tested with 2.4.0.1

Build
---
```
mkdir build-versatilepb
cd build-versatilepb
meson --cross-file ../meson.cross.file.txt --buildtype plain ..
ninja
```
Run
---
```
cd build-versatilepb
ninja run-test-putchar_dbg
```
