set -x #prints each command and its arguments to the terminal before executing it
# set -e #Exit immediately if a command exits with a non-zero status

sudo rmmod HW1_driver.ko
insmod HW1_driver.ko

./HW1_writer_arm64
