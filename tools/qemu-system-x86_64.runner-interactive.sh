#!/usr/bin/env bash
# Run a file with qemu
#
# Parameters:
#   $1 is the file to execute
set +x
#echo "param1=$1"
param1_filename=$1
shift
qemu-system-x86_64 $@ -nographic -no-reboot -drive format=raw,file=$param1_filename
#/home/wink/foss/qemu/build/x86_64-softmmu/qemu-system-x86_64 $@ -nographic -no-reboot -drive format=raw,file=$param1_filename
