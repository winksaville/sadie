#!/usr/bin/env bash
# Do two things:
#  1) Change the file to root for user and group
#  2) Make the file an SUID's i.e. allow it to run with the
#     permissions of the owner.

# Parameters:
#   $1 is the file to execute
echo "param1=$1"
param1=$1
shift
sudo chown root:root $param1
sudo chmod u+s $param1
