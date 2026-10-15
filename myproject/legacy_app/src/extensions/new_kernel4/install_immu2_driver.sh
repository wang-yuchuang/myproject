#!/bin/bash
insmod /home/driver/immu2/dido.ko
insmod /home/driver/immu2/tps3823_wdt.ko
insmod /home/driver/immu2/rtc-pcf8563.ko
# echo 0 > /sys/class/backlight/backlight/brightness
usleep 500000
insmod /home/driver/immu2/musb_am335x.ko
insmod /home/driver/immu2/musb_dsps.ko
# insmod /home/driver/usb_wwan.ko
# insmod /home/driver/option.ko
###Jimmy note: already put in kernel
#insmod /home/driver/usbnet.ko
insmod /home/driver/cdc_ether.ko
#Jimmy: now open the LCD power(without brightness)
echo 63 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio63/direction
echo 0 > /sys/class/gpio/gpio63/value
##Make alias
ln -s /dev/ttyO0 /dev/ttyS0	# Console Port
ln -s /dev/ttyO2 /dev/ttyS1	# RS232-1 Port
ln -s /dev/ttyO2 /dev/ttyS5	# RS232-1 Port Compatiable for IMMU1
ln -s /dev/ttyO3 /dev/ttyS2	# RS232-2 Port
ln -s /dev/ttyO3 /dev/ttyS6	# RS232-2 Port Compatiable for IMMU1
ln -s /dev/ttyO1 /dev/ttyS3	# RS485-1 Port
ln -s /dev/ttyO4 /dev/ttyS4	# RS485-2 Port
ln -s /dev/ttyO4 /dev/ttyS7	# ESAM Communication Port
