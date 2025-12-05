#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
# set -e #Exit immediately if a command exits with a non-zero status

rmmod -f mydev
insmod lab4_driver.ko

./lab4_writer_arm64 ERICWANG & #run in subshell
./lab4_reader_arm64 192.168.222.100 8000 /dev/mydev
