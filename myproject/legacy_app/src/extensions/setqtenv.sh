#!/bin/bash
##added by Jimmy 20200310: found this folder not included
PATH=$PATH:/usr/sbin
export QTDIR=/usr/lib/qt4
export TSDIR=/usr/lib/tslib
export PATH=$TSDIR/bin:$PATH
export LD_LIBRARY_PATH=/lib:/usr/lib:$TSDIR/lib:$QTDIR/lib
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0

#Jimmy added for CAP_LCD
#IS_EGTOUCH_CAP=`cat /proc/bus/input/devices | grep eGalax`
# IS_EGTOUCH_CAP=`dmesg | grep eGalaxTouch`
# #IS_EGTOUCH_RUN=`ps | grep eGTouchD | grep -v grep`
# #echo The LCD is:$IS_EGTOUCH_CAP
# #if [[ "$IS_EGTOUCH_CAP" != "" || "$IS_EGTOUCH_RUN" != "" ]]
# #if [ "$IS_EGTOUCH_RUN" != "" ]
# if [ "$IS_EGTOUCH_CAP" != "" ]
# then
# 	#echo "This is eGTouch CAP LCD..."
# 	export TSLIB_TSDEVICE=/dev/input/event1 
# 	export QWS_MOUSE_PROTO=Tslib:/dev/input/event1
# 	#if not run,then run it
# 	#test -z $IS_EGTOUCH_RUN && /usr/bin/eGTouchD&
# else
# 	export TSLIB_TSDEVICE=/dev/input/event0
# 	export QWS_MOUSE_PROTO=Tslib:/dev/input/event0
# 	#if run, kill it
# 	#test -z $IS_EGTOUCH_RUN && killall -9 eGTouchD
# fi

#IS_EGTOUCH_CAP=`dmesg | grep eGalaxTouch`
IS_ILITEK_CAP=`dmesg | grep ILITEK`
#IS_EGTOUCH_RUN=`ps | grep eGTouchD | grep -v grep`
#echo The LCD is:$IS_EGTOUCH_CAP
#if [[ "$IS_EGTOUCH_CAP" != "" || "$IS_EGTOUCH_RUN" != "" ]]
#if [ "$IS_EGTOUCH_RUN" != "" ]
#if [[ "$IS_EGTOUCH_CAP" != "" || "$IS_ILITEK_CAP" != "" ]]
if [ "$IS_ILITEK_CAP" != "" ]
then
        export TSLIB_TSDEVICE=/dev/input/event0 
        export QWS_MOUSE_PROTO=Tslib:/dev/input/event0
elif test -e /dev/input/touchscreen0; then
        #echo "This is eGTouch CAP LCD..."
        export TSLIB_TSDEVICE=/dev/input/touchscreen0 
        export QWS_MOUSE_PROTO=Tslib:/dev/input/touchscreen0
        #if not run,then run it
        #test -z $IS_EGTOUCH_RUN && /usr/bin/eGTouchD&
elif test -e /dev/input/event1; then
        export TSLIB_TSDEVICE=/dev/input/event1
        export QWS_MOUSE_PROTO=Tslib:/dev/input/event1
        #if run, kill it
        #test -z $IS_EGTOUCH_RUN && killall -9 eGTouchD
        #######Jimmy: do not use /dev/event0 any longer
#elif test -e /dev/event0; then
#        export TSLIB_TSDEVICE=/dev/event0
#        export QWS_MOUSE_PROTO=Tslib:/dev/event0
#        #if run, kill it
#        #test -z $IS_EGTOUCH_RUN && killall -9 eGTouchD
else
        export TSLIB_TSDEVICE=/dev/input/event0
        export QWS_MOUSE_PROTO=Tslib:/dev/input/event0
fi

######export TSLIB_TSDEVICE=/dev/input/event1
export TSLIB_PLUGINDIR=$TSDIR/lib/ts
export TSLIB_CONFFILE=$TSDIR/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
####export QWS_MOUSE_PROTO=Tslib:/dev/input/event1
export QT_QWS_FONTDIR=$QTDIR/lib/fonts
export QT_PLUGIN_PATH=$QTDIR/plugins/

