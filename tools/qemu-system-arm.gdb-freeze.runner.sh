#!/usr/bin/env bash
# $1 is the 'machine' to emulate. 'qemu-system-arm -M ?' for a list of machines
# $2 is the bin file to execute
qemu-system-arm -s -S -cpu arm1176 -M $1 -m 128M -nographic -no-reboot -kernel $2
