#!/usr/bin/env bash
# Do two things:
#  1) Copy the input to output
#  1) Change user and group for the output file to root
#  2) SUID the output file (i.e. have it run with the permissions of the owner.)

# Parameters:
#   $1 is the input file to execute
input=$1
shift
output=$1
shift
echo "input=$input"
echo "output=$output"

cp $input $output
sudo chown root:root $output
sudo chmod u+s $output
