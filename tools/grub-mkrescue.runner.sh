#!/usr/bin/env bash
# Run grub-mkrescue to create an iso.img file
#
# Parameters:
#   $1 is the file to execute
#   $2 grub.cfg file
#   $3 iso image file
tmpdir=$(mktemp -d /tmp/grub-mkrescue.isofiles.XXX)
echo "'tmpdir=${tmpdir}"
mkdir -p ${tmpdir}/boot/grub
cp $1 ${tmpdir}/boot/
cp $2 ${tmpdir}/boot/grub/grub.cfg
grub-mkrescue -o $3 ${tmpdir} 2> /dev/null
rm -rf ${tmpdir}
