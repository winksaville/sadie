#!/usr/bin/env bash
# Run grub-mkrescue to create an iso.img file
#
# Parameters:
#   $1 is the file to execute
#   $2 grub.cfg file
#   $3 iso image file
#set -x
tmpdir=$(mktemp -d /tmp/grub-mkrescue.isofiles.XXX)
#echo "'tmpdir=${tmpdir}"
mkdir -p ${tmpdir}/boot/grub
cp $1 ${tmpdir}/boot/
cp $2 ${tmpdir}/boot/grub/grub.cfg
# Turns out grub-mkresule doesn't report an error if the input file doesn't
# exist. I posted an email to help-grub@gnu.org to find out why, but that
# probably means ther might be other reasons a failure won't be reported
# so this test may not be useful.
grub-mkrescue -d /usr/lib/grub/i386-pc/ -o $3 ${tmpdir} 2> /dev/null || { echo "grub-mkrescue fialed with $?"; exit 1; }
rm -rf ${tmpdir}
