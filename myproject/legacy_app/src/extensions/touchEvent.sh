#!/bin/sh
PATH=$PATH:/usr/sbin
#echo Setting event0 to touchscreen0 for Qt touch apps.
echo "#######NEW_Build /dev/input for Qt apps########"
mkdir -p /dev/input
#rm -rf /dev/event0
#rm -rf /dev/input/event0
#rm -rf /dev/input/event1
rm -rf /dev/input/touchscreen0
#ln -sf /dev/input/event0 /dev/input/touchscreen0