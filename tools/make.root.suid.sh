#!/usr/bin/env bash
# Do two things:
#  1) Change user and group for the file to root
#  2) SUID the file (i.e. have it run with the permissions of the owner.)

# Parameters:
#   $1 is the file to execute
echo "param1=$1"
param1=$1
shift
sudo chown root:root $param1
sudo chmod u+s $param1
