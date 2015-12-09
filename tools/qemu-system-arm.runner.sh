#!/usr/bin/env bash
# This is used to run an application using qemu and then check if
# the output contains 'ERROR: expr' which is output by AC_TEST on
# errors. The number of occurances of 'ERROR: expr' becomes the exit
# code. Thus if no errors then the exit code it 0. I needed to do this
# because I couldn't find a way to have qemu exit with a error code
# coming from the application being run.
#
# Parameters:
#   $1 is the 'machine' to emulate. 'qemu-system-arm -M ?' for a list of machines
#   $2 is the bin file to execute
tmpfile=$(mktemp /tmp/sadie-qemu-system-arm.runner.XXX)
qemu-system-arm -cpu arm1176 -M $1 -m 128M -nographic -no-reboot -kernel $2 </dev/null 2>&1 | tee ${tmpfile}
err_count=$(grep -c -e 'ERROR: expr' ${tmpfile})
#echo err_count=${err_count}
((${err_count} == 0)) && rm ${tmpfile}
exit ${err_count}
