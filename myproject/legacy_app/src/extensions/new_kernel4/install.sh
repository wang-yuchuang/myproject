#!/bin/sh

EXTENSION_FOLDER=/app/extensions
SCRIPT_PATH="/home/app_script"
APP_FOLDER=/app
#ARG_1=$1

EXE_NAND_ERASE=$EXTENSION_FOLDER/flash_erase
EXE_NAND_WRITE="/usr/sbin/nandwrite"
#if the tools not found, so we quit the process
if test -x $EXE_NAND_ERASE -a -x $EXE_NAND_WRITE
then
    :
else
    echo "not find nand_tool, so quit the kernel update..."
    exit 0
fi

#IS_IMMU2_ALREADY_RUNIAT_KERNEL_4=`uname -a | grep "4.19"`
#if test -n "$IS_IMMU2_ALREADY_RUNIAT_KERNEL_4"; then
#    echo "This is ALREADY KERNEL-4, so we SKIP upgrade......"
#    exit 0
#fi

###added by WLD01206 2024.12.18 modify mu2 resolution 800*480 -> 1280*800
LCD_10INCH="$APP_FOLDER/mark_10inchlcd_flag.txt"
LCD_10INCH_FLAG=0

if test -e $LCD_10INCH; then
    LCD_10INCH_MARK="/home/mark_10inchlcd_flag.txt"
    if test -e $LCD_10INCH_MARK; then
        LCD_10INCH_FLAG=0
    else
        cp -f $APP_FOLDER/mark_10inchlcd_flag.txt /home/
        LCD_10INCH_FLAG=1
    fi
    rm $APP_FOLDER/mark_10inchlcd_flag.txt
fi

IS_IMMU2_A02=`hostname | grep "\-a02"`
if test -n "$IS_IMMU2_A02"; then
    ###added by Jimmy 20240529 to fix the Uboot 600MHz -> 800MHz issue
    IS_IMMU2_ON800MHZ=`cat /sys/kernel/debug/clk/dpll_mpu_ck/clk_rate | grep "800000"`
    EXIT_RTN=0
    if ! test -n "$IS_IMMU2_ON800MHZ"; then
        echo "Now ONLINE updating Uboot for IMMU2..."
        NEW_UBOOTIMG=$EXTENSION_FOLDER/new_kernel4/u-boot.img
        NEW_MLO=$EXTENSION_FOLDER/new_kernel4/MLO
        if [[ -e $NEW_UBOOTIMG ]] && [[ -e $NEW_MLO ]]; then
            $EXE_NAND_ERASE  /dev/mtd0 0 0
            $EXE_NAND_WRITE -p /dev/mtd0 $NEW_MLO  
            echo "MLO upgrade done..."
            $EXE_NAND_ERASE  /dev/mtd5 0 0
            $EXE_NAND_WRITE -p /dev/mtd5 $NEW_UBOOTIMG  
            echo "UBOOT upgrade done..."
            EXIT_RTN=1
        fi     
    fi

    ###changed by Jimmy for adding new support of LTEmodem
    cp -f $EXTENSION_FOLDER/new_kernel4/option.ko /home/driver/
    cp -f $EXTENSION_FOLDER/new_kernel4/usb_wwan.ko /home/driver/   
    if [ $LCD_10INCH_FLAG -eq 0 ]; then
        ##IS_ALREDY_NEW=`dmesg | grep "kernel" | grep "kernel_A00T07_USBTouchDriver_MoveModule"`
        IS_ALREDY_NEW=`dmesg | grep "kernel" | grep "kernel_20220627_SupportNewPMIC"`
        if test -n "$IS_ALREDY_NEW"; then      
            IS_DIDO_WRONG=`dmesg | grep "dido" | grep "disagrees about version"` 
            if test -z "$IS_DIDO_WRONG"; then 
                echo "kernel already up-2-date, so skip upgrading..."
                if [ "$EXIT_RTN" -gt 0 ]
                then
                    exit 1
                else
                    exit 0
                fi
                exit 0
            fi
        fi
        FILE_DTB_NEW=$EXTENSION_FOLDER/new_kernel4/am335x-immu2-rm-PMIC-i2c-comm-20220630.dtb
    else
        FILE_DTB_NEW=$EXTENSION_FOLDER/new_kernel4/am335x-immu3g_A02_10_lcd_60Hz.dtb
    fi
    
    ##now copy the needed files into FS    
    cp -f $EXTENSION_FOLDER/new_kernel4/usbnet.ko /home/driver/
    cp -f $EXTENSION_FOLDER/new_kernel4/cdc_ether.ko /home/driver/
    cp -f $EXTENSION_FOLDER/new_kernel4/dido.ko /home/driver/immu2/
    cp -f $EXTENSION_FOLDER/new_kernel4/tps3823_wdt.ko /home/driver/immu2/
    cp -f $EXTENSION_FOLDER/new_kernel4/rtc-pcf8563.ko /home/driver/immu2/
    cp -f $EXTENSION_FOLDER/new_kernel4/hid-multitouch.ko /home/driver/immu2/
    cp -f $EXTENSION_FOLDER/new_kernel4/musb_am335x.ko /home/driver/immu2/
    cp -f $EXTENSION_FOLDER/new_kernel4/musb_dsps.ko /home/driver/immu2/
    cp -f $EXTENSION_FOLDER/new_kernel4/install_immu2_driver.sh /home/app_script/
    cp -f $EXTENSION_FOLDER/new_kernel4/disable_wd /home/app_script/
    chmod +x /home/app_script/* -R
    echo "COPY NEW fs FILES done..."
    echo "This is for A02 new kernel4..."
    FILE_KERNEL_NEW=$EXTENSION_FOLDER/new_kernel4/zImage-rm-cpuFreqScaling-20220630
    
    if [[ -e $FILE_KERNEL_NEW ]] && [[ -e $FILE_DTB_NEW ]]; then
        $EXE_NAND_ERASE  /dev/mtd8 0 0
        $EXE_NAND_WRITE -p /dev/mtd8 $FILE_KERNEL_NEW  
        echo "kernel-4 upgrade done..."
        $EXE_NAND_ERASE  /dev/mtd4 0 0
        $EXE_NAND_WRITE -p /dev/mtd4 $FILE_DTB_NEW  
        echo "dtb-4 upgrade done..."
        exit 1
    fi

    if [ "$EXIT_RTN" -gt 0 ]
    then
        exit 1
    fi
fi

exit 0
