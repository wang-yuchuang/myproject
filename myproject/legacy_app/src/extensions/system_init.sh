#!/bin/bash
###### FS version
#echo "###### IMMU2_A00/IMMU2_A01 file system verison: V1.04_20190320 ########"
test -x /etc/init.d/hwclock.sh && /etc/init.d/hwclock.sh start
if test -e /etc/timestamp
then
        SYSTEMDATE=`date -u +%4Y%2m%2d%2H%2M`
        read TIMESTAMP < /etc/timestamp
        if [ ${TIMESTAMP} -gt $SYSTEMDATE ]; then
                date -u ${TIMESTAMP#????}${TIMESTAMP%????????}
#                test -x /etc/init.d/hwclock.sh && /etc/init.d/hwclock.sh stop
        fi
fi

ip link set can0 type can restart-ms 100
ip link set can1 type can restart-ms 100
#ip link set can0 type can bitrate 125000 triple-sampling on
#ip link set can1 type can bitrate 125000 triple-sampling on
####'triple-sampling' is NO longer supported in new Kernel
ip link set can0 type can bitrate 125000
ip link set can1 type can bitrate 125000
ip link set can0 up 
ip link set can1 up 

####Jimmy added for CAP_LCD judge
IS_EGTOUCH_CAP=`dmesg | grep eGalaxTouch`
if [ "$IS_EGTOUCH_CAP" != "" ]
then
  /usr/bin/eGTouchD&
  #sleep 1
  #if not succeded, then restart it
  #IS_EGTOUCH_RUN=`ps | grep eGTouchD | grep -v grep | wc -l`
  #if test $IS_EGTOUCH_RUN -le 0; then
  #    /usr/bin/eGTouchD&
  #    echo "Restart eGTouchD..."
  #fi
  #now wait CAP_LCD ready, max 10 sec
  wait_max=10  
  while [ $wait_max -ge 1 ]
  do
    #IS_CAP_REAADY=`cat /proc/bus/input/devices | grep eGalax`
    IS_CAP_REAADY=`ls /dev/input/ | grep touchscreen0` 
    if [[ -n "$IS_CAP_REAADY" ]]; then
      echo "cap_lcd init done........"
      break
    else
      echo "waiting for CAP_LCD init.........."
    fi
    let wait_max--
    sleep 1
  done
fi

###added by Jimmy for iliTek Cap-LCD
insmod /home/driver/immu2/hid-multitouch.ko

########### export QT ENV ################
source /home/app_script/setqtenv.sh

########### QT should be started here ####

### start app and qt here ####
/app/startrun

