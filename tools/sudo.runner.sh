#!/usr/bin/env bash
# This is used to run an application using sudo
#
# Parameters:
#   $1 is the file to execute
echo "executable=$1"
executable=$1
shift
sudo $executable $@
