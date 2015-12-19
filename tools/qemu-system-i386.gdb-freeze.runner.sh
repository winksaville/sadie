#!/usr/bin/env bash
# $1 is the file to execute
qemu-system-i386 -s -S -nographic -no-reboot -kernel $1
