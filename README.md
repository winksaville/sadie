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
* [binutils-gdb](https://www.gnu.org/software/binutils/) Tested with ToT from git repo
* [Gcc](https://gcc.gnu.org/) Tested with 5.2
* [qemu](http://wiki.qemu.org/Main_Page) Tested with 2.4.0.1

Create cross-tool-chain
---
Create a directory for getting and building the tools
```
mkdir -p ~/foss/gcc-tool-chain
mkdir -p ~/foss/gcc-tool-chain/downloads
mkdir -p ~/foss/gcc-tool-chain/builds
cd ~/foss/gcc-tool-chain
```
Clone binutils and gdb from their git repo. Then
build and install them from the out-of-tree directory
builds/ previously created.
```
git clone git://sourceware.org/git/binutils-gdb.git
cd builds
mkdir -p ~/foss/gcc-tool-chain/builds/binutils-gdb-arm-eabi
cd ~/foss/gcc-tool-chain/builds/binutils-gdb-arm-eabi
../../gdb/configure --prefix=$HOME/opt/cross --target=arm-eabi --disable-nls
make all -j4
make install
```
We'll download the gcc sources, gmp, mpfr and mpc via the web.
and untar them to there respective directories
```
cd ~/foss/gcc-tool-chain/downloads
wget http://ftp.gnu.org/gnu/gcc/gcc-5.2.0/gcc-5.2.0.tar.bz2
wget https://gmplib.org/download/gmp/gmp-6.0.0a.tar.xz
wget http://www.mpfr.org/mpfr-current/mpfr-3.1.3.tar.xz
wget ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
cd ..
tar -xvf downloads/gcc-5.2.0.tar.bz2
tar -xvf downloads/gmp-6.0.0a.tar.xz
tar -xvf downloads/mpfr-3.1.3.tar.xz
tar -xvf downloads/mpc-1.0.3.tar.gz
```
Make symbolic links in gcc to the multi-precision libraries that have been downloaded.
```
cd gcc-5.2.0
ln -s ../gmp-6.0.0 gmp
ln -s ../mpfr-3.1.3 mpfr
ln -s ../mpc-1.0.3 mpc
cd ..
```
Then configure and build gcc c and c++ and we also specify --without-headers as this is a bare metal compiler.
```
mkdir -p ~/foss/gcc-tool-chain/builds/gcc-5.2.0-arm-eabi
cd ~/foss/gcc-tool-chain/builds/gcc-5.2.0-arm-eabi
../../gcc-5.2.0/configure --prefix=$HOME/opt/cross --target=arm-eabi --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j4
make install-gcc
make all-target-libgcc -j4
make install-target-libgcc
```
Get the sources for this project from github
```
mkdir -p ~/prgs
cd ~/prgs
git https://github.com/winksaville/xx1-kernel.git
cd xx1-kernel
```
Build xx1-kernel
---
```
mkdir -p ~/prgs/xx1-kernel/build-versatilepb
cd ~/prgs/xx1-kernel/build-versatilepb
meson --cross-file ../meson.cross.file.txt --buildtype plain ..
ninja
```
Run
---
```
ninja run-test-putchar_dbg
```
