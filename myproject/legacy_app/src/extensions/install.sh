#!/bin/sh

EXTENSION_FOLDER=/app/extensions
# SCRIPT_PATH="/home/app_script"
# #ARG_1=$1
# IS_IMMU2_ALREADY_RUNIAT_KERNEL_4=`uname -a | grep "4.19"`

# function inst_option_ko()
# {
#     if test -n "$IS_IMMU2_ALREADY_RUNIAT_KERNEL_4"; then
#         return 0
#     fi    
#     echo "Install the new optinon.ko"
#     cp $EXTENSION_FOLDER/option.ko /home/driver -f

#     #need reboot, so return true = 0, false = 1
#     return 0
# }

# function inst_caplcd_driver()
# {
#     echo "Install the eGTouch LCD driver"
#     USR_BIN="/usr/bin"    

#     cp $EXTENSION_FOLDER/eGTouchD $USR_BIN -f
#     chmod +x $USR_BIN/eGTouchD
#     cp $EXTENSION_FOLDER/eGTouchL.ini /etc -f
#     #change sytem_init script to support both cap/resist LCD
#     cp $EXTENSION_FOLDER/setqtenv.sh $SCRIPT_PATH -f
#     chmod +x $SCRIPT_PATH/setqtenv.sh
#     #cp /app/extensions/system_init.sh $SCRIPT_PATH -f
#     #IS_AUTOBOOT_APP=`cat /home/app_script/system_init.sh | grep -c "#/app/startrun"`
#     #if [ "$IS_AUTOBOOT_APP" -lt 1 ]
#     #then 
#     cp $EXTENSION_FOLDER/system_init.sh $SCRIPT_PATH -f
#     chmod +x $SCRIPT_PATH/system_init.sh
#     #else
#     #    echo "In debug mode, so do not update the system_int.sh file"
#     #fi

#     #need reboot, so return true = 0, false = 1
#     #jimmy, need remove this file as it is not needed any more
#     rm /etc/profile.d/tslib.sh -f
#     return 0
# }

# #function inst_new_kernel()
# ##{    
# #    if test -n "$IS_IMMU2_ALREADY_RUNIAT_KERNEL_4"; then
# #        echo "This is ALREADY KERNEL-4, so we SKIP kernel-3 upgrade......"
# #        return 1
# #    fi
# #
# #    EXE_NAND_ERASE=$EXTENSION_FOLDER/flash_erase
# #    EXE_NAND_WRITE="/usr/sbin/nandwrite"
# #    #if the tools not found, so we quit the process
# #    if test -x $EXE_NAND_ERASE -a -x $EXE_NAND_WRITE
# #    then
# #        :
# #    else
# #        echo "not find nand_tool, so quit the kernel update..."
# #        return 1
# #    fi
# #
# #    IS_IMMU2_A01=`hostname | grep "\-a01"`
# #    IS_IMMU2_A02=`hostname | grep "\-a02"`
# #    if test -n "$IS_IMMU2_A01"; then
# #        echo "This is for A01..."
# #        IS_EXISTED_KERNEL_NEW=`dmesg | grep "kernel" | grep "V1.01T11 with USB/DCAN/ETH"`
# #        FILE_KERNEL_NEW=$EXTENSION_FOLDER/zImage_eth_usb_can_patch_A01
# #        if [[ "$IS_EXISTED_KERNEL_NEW" = "" ]] && [[ -e $FILE_KERNEL_NEW ]]; then
# #            $EXE_NAND_ERASE  /dev/mtd8 0 0
# #            $EXE_NAND_WRITE -p /dev/mtd8 $FILE_KERNEL_NEW  
# #            echo "kernel upgrade done..."
# #            return 0
# #        fi
# #    elif test -n "$IS_IMMU2_A02"; then
# #        echo "This is for A02..."
# #        #Jimmy note 20200720: new dma kernel not work with modem, so try with old kernel
# #        #IS_EXISTED_KERNEL_NEW=`dmesg | grep "kernel" | grep "V1.01T12_T02 test USB without logo"`
# #        #FILE_KERNEL_NEW=$EXTENSION_FOLDER/zImage_usb_dma_disabled_A02
# #        IS_EXISTED_KERNEL_NEW=`dmesg | grep "kernel" | grep "V1.01T12 without boot logo"`
# #        FILE_KERNEL_NEW=$EXTENSION_FOLDER/zImage_eth_usb_can_patch_A02
# #        if [[ "$IS_EXISTED_KERNEL_NEW" = "" ]] && [[ -e $FILE_KERNEL_NEW ]]; then
# #            $EXE_NAND_ERASE  /dev/mtd8 0 0
# #            $EXE_NAND_WRITE -p /dev/mtd8 $FILE_KERNEL_NEW  
# #            echo "kernel upgrade done..."
# #            return 0
# #        fi
# #    fi
# #
# #    return 1
# #}

# #note 0=false, 01=true
# IS_NEED_REBOOT=0

# #inst_option_ko
# #test $? -eq 0 && IS_NEED_REBOOT=1

# #inst_caplcd_driver
# #test $? -eq 0 && IS_NEED_REBOOT=1


# #2021_07_03 Remove By John, ECS SW Not Support HW-A01 Version
# #inst_new_kernel
# #test $? -eq 0 && IS_NEED_REBOOT=1

# #added by Jimmy 20220411 for eGTouchD driver update
# IS_EGTOUCHD_NEW=`eGTouchD -v | grep -c '2.5.10703'`
# ##########Jimmy 20210208: 2.5.93 not work, so roll back to 2.5.74
# #############IS_EGTOUCHD_NEW=`eGTouchD -v | grep -c '2.5.74'`

# if [ "$IS_NEED_REBOOT" -gt 0 ]
# then  
#     inst_option_ko
#     inst_caplcd_driver
# elif [ "$IS_EGTOUCHD_NEW" -lt 1 ]
# then
#     inst_caplcd_driver
#     IS_NEED_REBOOT=1
# else
#     inst_caplcd_driver
# fi

# #added by Jimmy 20210322 for touchScreen driver
# IS_TOUCHEVSH_NEED_UPDT=`cat /etc/init.d/touchEvent.sh | grep -c "####NEW_Build"`
# if [ "$IS_TOUCHEVSH_NEED_UPDT" -lt 1 -o "$IS_NEED_REBOOT" -gt 0 ]; then
#     cp $EXTENSION_FOLDER/touchEvent.sh /etc/init.d/ -f
#     chmod +x /etc/init.d/touchEvent.sh
#     IS_NEED_REBOOT=1
# fi

# #added by Jimmy 20210204 for hid-multitouch.ko
# HID_DRIVER_FILE=/home/driver/immu2/hid-multitouch.ko
# if ! [ -f $HID_DRIVER_FILE ]; then
#     ########do not upgrade it for kernel-4
#     if test -n "$IS_IMMU2_ALREADY_RUNIAT_KERNEL_4"; then
#         :
#     fi    
#     cp $EXTENSION_FOLDER/hid-multitouch.ko /home/driver/immu2 -f
#     IS_NEED_REBOOT=1
# fi

# #added by Jimmy 20200620 for resolving the CAN bitrate error bug
# IS_AUTOBOOT_APP=`cat /home/app_script/system_init.sh | grep -c "bitrate 125000"`
# if [ "$IS_AUTOBOOT_APP" -lt 1 ]
# then 
#     cp $EXTENSION_FOLDER/system_init.sh $SCRIPT_PATH -f
#     chmod +x $SCRIPT_PATH/system_init.sh
# else
#     echo "The CAN biterror is already resolved!"
# fi

# #added by Jimmy 20201106 to update libcurl
# IS_CURL_SUPPORT_FTPS=`curl -V | grep -c 'ftps'`
# if [ "$IS_CURL_SUPPORT_FTPS" -lt 1 ]
# then 
#     cp $EXTENSION_FOLDER/libcurl.so /usr/lib/ -f
#     cp /usr/lib/libcurl.so /usr/lib/libcurl.so.4 -f
#     cp /usr/lib/libcurl.so /usr/lib/libcurl.so.4.5.0 -f
#     chmod +x -R /usr/lib/libcurl.so*
# else
#     echo "libcurl is already updated!"
# fi

# #added by Jimmy 20201221 for LCD power ctrl
# IS_DIDO_DRIVER_NEW=`dmesg | grep "DIDO" | grep -c "V1.05_T03"`
# ########do not upgrade it for kernel-4
# if test -n "$IS_IMMU2_ALREADY_RUNIAT_KERNEL_4"; then
#     :
# else  
#     if [ "$IS_DIDO_DRIVER_NEW" -lt 1 ]
#     then 
#         IS_DIDO_DRIVER_NEW_4=`dmesg | grep "DIDO" | grep -c "V2.00"`
#         if [ "$IS_DIDO_DRIVER_NEW" -lt 1 ]
#         then
#             cp $EXTENSION_FOLDER/dido.ko /home/driver/immu2/ -f
#             chmod +x /home/driver/immu2/dido.ko
#         else    
#             echo "DIDO driver is V2.00 so we skip upgrade!"
#         fi
#     else
#         echo "DIDO driver is already updated!"
#     fi
# fi 
# #####added by Jimmy 20210415 for installing libnopoll
# test -e /usr/lib/libnopoll.so.0 || cp -f $EXTENSION_FOLDER/libnopoll.so /usr/lib/libnopoll.so.0

# #echo "---Judge libpaho-mqtt3a* is Exsit---"
# #test -e /usr/lib/libpaho-mqtt3a.so || cp -f $EXTENSION_FOLDER/new_kernel4/libpaho-mqtt3a.so /usr/lib/
# #test -e /usr/lib/libpaho-mqtt3a.so.1 || cp -f $EXTENSION_FOLDER/new_kernel4/libpaho-mqtt3a.so.1 /usr/lib/
# #test -e /usr/lib/libpaho-mqtt3a.so.1.3 || cp -f $EXTENSION_FOLDER/new_kernel4/libpaho-mqtt3a.so.1.3 /usr/lib/
# #test -e /usr/lib/libpaho-mqtt3as.so || cp -f $EXTENSION_FOLDER/new_kernel4/libpaho-mqtt3as.so /usr/lib/
# #test -e /usr/lib/libpaho-mqtt3as.so.1 || cp -f $EXTENSION_FOLDER/new_kernel4/libpaho-mqtt3as.so.1 /usr/lib/
# #test -e /usr/lib/libpaho-mqtt3as.so.1.3 || cp -f $EXTENSION_FOLDER/new_kernel4/libpaho-mqtt3as.so.1.3 /usr/lib/
# #echo "---cp libpaho-mqtt3a* to /usr/lib/---"

# if [ -e /usr/lib/libpaho-mqtt3as.so.1.3 -a -e /usr/lib/libpaho-mqtt3a.so.1.3 ];then
# echo "Exist mqtt3as.so.1.3 .Need Update libpaho.so"
# rm /usr/lib/libpaho-mqtt3*
# cp -f $EXTENSION_FOLDER/libpaho-mqtt3a.so.1 /usr/lib/
# cp -f $EXTENSION_FOLDER/libpaho-mqtt3as.so.1 /usr/lib/
# else
# echo "No Need Update libpaho.so"
# test -e /usr/lib/libpaho-mqtt3a.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3a.so.1 /usr/lib/libpaho-mqtt3a.so.1
# test -e /usr/lib/libpaho-mqtt3as.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3as.so.1 /usr/lib/libpaho-mqtt3as.so.1
# fi

# cp -f $EXTENSION_FOLDER/libpaho-mqtt3a.so.1 /usr/lib/libpaho-mqtt3a.so.1
# cp -f $EXTENSION_FOLDER/libpaho-mqtt3as.so.1 /usr/lib/libpaho-mqtt3as.so.1

# test -e /usr/lib/libpaho-mqtt3c.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3c.so.1 /usr/lib/libpaho-mqtt3c.so.1
# test -e /usr/lib/libpaho-mqtt3cs.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3cs.so.1 /usr/lib/libpaho-mqtt3cs.so.1

# ########add keepalived tool########
# if [ ! -d /etc/keepalived ]; then
#     mkdir -p /etc/keepalived
#     echo "Directory /etc/keepalived created."
# fi
# test -e /app/extensions/keepalived && cp -f /app/extensions/keepalived /usr/bin/keepalived && chmod 0755 /usr/bin/keepalived
# #test -e /etc/keepalived/keepalived.conf || { test -e /app/extensions/keepalived.conf && cp -f /app/extensions/keepalived.conf /etc/keepalived/keepalived.conf && chmod 0644 /etc/keepalived/keepalived.conf; }
# #test -e /app/extensions/keepalived.conf && cp -f /app/extensions/keepalived.conf /etc/keepalived/keepalived.conf && chmod 0644 /etc/keepalived/keepalived.conf
# test -e /app/extensions/notify.sh && cp -f /app/extensions/notify.sh /home/app_script/notify.sh && chmod 0755 /home/app_script/notify.sh

# KERNEL4_INSTALLER=$EXTENSION_FOLDER/new_kernel4/install.sh
# if test -e $KERNEL4_INSTALLER; then
#     chmod +x $KERNEL4_INSTALLER 
#     sh $KERNEL4_INSTALLER
#     test $? -eq 1 && IS_NEED_REBOOT=1
# fi

# #added by Jimmy 20210827 as ILTEK USBtouchscreen driver is conlict with the lusb(solved by updt kernel)
# LUSB_RUNPATH=/usr/bin/lsusb.bak
# if test -e $LUSB_RUNPATH; then
#     mv $LUSB_RUNPATH /usr/bin/lsusb
#     IS_NEED_REBOOT=1
# fi

# #######added by Jimmy 20210517 for SSH port re-alloacating
# IS_SSHPORT_OLD=`cat /etc/ssh/sshd_config | grep "#Port 22"`
# ########do not upgrade it for kernel-4
# if test -n "$IS_SSHPORT_OLD"; then
#     cp $EXTENSION_FOLDER/sshd_config /etc/ssh/sshd_config -f
#     IS_NEED_REBOOT=1
# fi  
# IS_ROOTPAS_NEW=`cat /etc/shadow | grep "nBnM:1876"`
# if test -z "$IS_ROOTPAS_NEW"; then
#     cp $EXTENSION_FOLDER/newpsd /etc/shadow -f
#     IS_NEED_REBOOT=1
# fi

# #####Finally, rm ext folder and judge if need reboot
# rm $EXTENSION_FOLDER -rf

# ###added by Jimmy 20200910 just to avoid file crash
# sync
# ###added by Jimmy 20200910 : move it tail of file
# if [ "$IS_NEED_REBOOT" -gt 0 ]
# then  
#     echo "reboot=1"
#     ####do not judge $1
#     #if [ -n "$1" ]; then
# 		#echo "reboot=1"
#         #exit 5
# 	#else
#     #    :
# 	    #touch $EXTENS_MARK_FILE    
# 		#reboot	
# 	#fi
# fi


# Libraries loaded via LD_LIBRARY_PATH ($EXTENSION_FOLDER) - no cp to /usr/lib needed
# test -e /usr/lib/libnopoll.so.0 || cp -f $EXTENSION_FOLDER/libnopoll.so /usr/lib/libnopoll.so.0
#
# test -e /usr/lib/libpaho-mqtt3a.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3a.so.1 /usr/lib/libpaho-mqtt3a.so.1
# test -e /usr/lib/libpaho-mqtt3as.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3as.so.1 /usr/lib/libpaho-mqtt3as.so.1
# test -e /usr/lib/libpaho-mqtt3c.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3c.so.1 /usr/lib/libpaho-mqtt3c.so.1
# test -e /usr/lib/libpaho-mqtt3cs.so.1 || cp -f $EXTENSION_FOLDER/libpaho-mqtt3cs.so.1 /usr/lib/libpaho-mqtt3cs.so.1
