#!/bin/bash
PAC_TYPE=$1
BOARD_NAME=$2
PROJECT_NAME=$3
PARTITION_MODE=$4
XIP_MODE=$5
sec_ver=$6
orig_iib_info=$7
sfs_name=$8
boot0_addr=$9
boot1_addr=${10}
boot2_addr=${11}
rfd_name=${12}
partition_file=${13}
fat_file=${14}

BOARD_SHORT_NAME=${BOARD_NAME}
OUTPUT_OSPI=pac/$BOARD_NAME/$PROJECT_NAME/ospi_$BOARD_SHORT_NAME.pac
OUTPUT_EMMC=pac/$BOARD_NAME/$PROJECT_NAME/emmc_$BOARD_SHORT_NAME.pac
DLOADER=dloader_$BOARD_SHORT_NAME.bin
SIGNED_BIN=_signed.bin
DLOADER_SIGNED=dloader_$BOARD_SHORT_NAME$SIGNED_BIN
# bpt header size#
bpt_sz=0x1000
xspi1_mem_base=0x10000000
boot_addr_array=("$boot0_addr" "$boot1_addr" "$boot2_addr")
enable_reserved=true

echo -e "pac type:" $PAC_TYPE
echo -e "board name:" $BOARD_NAME
echo -e "board short name:" $BOARD_SHORT_NAME
echo -e "project name:" $PROJECT_NAME
echo -e "parition mode:" $PARTITION_MODE
echo -e "xip mode:" $XIP_MODE
echo -e "sec_ver:" ${sec_ver}
echo -e "orig_iib_info:" ${orig_iib_info}
echo -e "sfs_name:" ${sfs_name}
echo -e "boot0_addr:" ${boot0_addr}
echo -e "boot1_addr:" ${boot1_addr}
echo -e "boot2_addr:" ${boot2_addr}
echo -e "rfd_name:" ${rfd_name}
echo -e "partition_file:" ${partition_file}

echo -e "output flash pac:" $OUTPUT_OSPI
echo -e "output emmc pac:" $OUTPUT_EMMC
echo -e "dloader name:" $DLOADER
echo -e "dloader signed name:" $DLOADER_SIGNED

chmod 777 */*
if [ "$PAC_TYPE" = "ospi" ];then
rm $OUTPUT_OSPI -f
fi
if [ "$PAC_TYPE" = "emmc" ];then
rm $OUTPUT_EMMC -f
fi
rm temp/*.* -f
cp -r bin/. temp

if [ ! -d pac/$BOARD_NAME ]; then
 mkdir -p pac/$BOARD_NAME
fi

if [ "$BOARD_NAME" == "e3_disp" ]; then
    enable_reserved=false
fi

pad_image_to_reserved()
{
    local all_iib
    local iib_image_info
    #0:sf, 2:sp0 split,3:sp1 split, 4:sx0 split,5:sx1 split,6:sx lockstep, 7:sp lockstep#
    local image_reserved_size=(0x80000 0x40000 0x40000 0x40000 0x40000 0x40000 0x40000 0x40000)
    local image_size_temp
    local core_type
    local file_sz
    local file

    all_iib=$1

    while [ -n "$all_iib" ]
    do
        iib_info=${all_iib##*--iib}
        core_type=
        for element in ${iib_info[*]}
        do
            echo $element | grep "core=" > /dev/null
            if [[ $? -eq 0 ]];then
                core_type=${element##*=}
                continue
            fi

            echo $element | grep "image=" > /dev/null
            if [[ $? -eq 0 ]];then
                iib_image_info=${element}
                continue
            fi

            echo $element | grep "to=" > /dev/null
            if [[ $? -eq 0 ]];then
                continue
            fi

            echo $element | grep "entry=" > /dev/null
            if [[ $? -eq 0 ]];then
                continue
            fi
        done
        if [[ x"$iib_image_info" != x"" ]];then
            file=${iib_image_info##*=}
            file_sz=`stat --format=%s $file`
            test $? -ne 0 && exit 1
            echo "file sz:"${file_sz}
            image_size_temp=${image_reserved_size[$core_type]}
            image_size_temp=$(printf '%d' $image_size_temp)
            echo "core:$core_type, pad:"$image_size_temp
            if [ $file_sz -gt $image_size_temp ];then
                echo "${file} size is larger than reserved, file size:$file_sz, reserved size:${image_reserved_size[$core_type]}"
                exit 1
            fi
            file_sz=$image_size_temp
            echo "truncate $file to $file_sz"
            file_sz=$(printf '%d' $file_sz)
            truncate -s ${file_sz} ${file}
        fi
        all_iib=${all_iib%--*}
        #trim space
        all_iib=`eval echo $all_iib`
    done
}

calc_entry()
{
    local iib_info addr_base
    local iib_info_array=()
    local i=0
    local all_iib
    local ret_ptr
    local orig_addr_base

    addr_base=$1
    all_iib=$2
    ret_ptr=$3

    printf 'The first image addr_base:0x%x\n' $addr_base
    orig_addr_base=$addr_base

    while [ -n "$all_iib" ]
    do
        iib_info=${all_iib##*--iib}
        #echo "old iib:"$iib_info
        all_iib=${all_iib%--*}
        all_iib=`eval echo $all_iib`

        iib_info_array[i]="$iib_info"
        ((i++))
    done


    local file_sz=
    local file=
    local new_iib=
    local new_to=
    local new_entry=
    local pad_sz=4096 #For BootROM,it needs to pad image to 512 Bytes, but for remap it needs to pad image to 4KB.
    local mod=0

    ((i--))
    while [[ $i -ge 0 ]]
    do
        iib_info=${iib_info_array[i]}
        echo "--------------------------------------------"
        echo "old_iib:"${iib_info}
        new_iib+=" --iib"
        for element in ${iib_info[*]}
        do
            echo $element | grep "image=" > /dev/null
            if [[ $? -eq 0 ]];then
                file=${element##*=}
                file_sz=`stat --format=%s $file`
                test $? -ne 0 && exit 1
                echo "file sz:"${file_sz}
                mod=$((file_sz % pad_sz))
                if [ $mod -gt 0 ];then
                    file_sz=$((file_sz + pad_sz - mod))
                    truncate -s ${file_sz} ${file}
                    echo "pad file sz:"${file_sz}
                fi
                printf 'addr_base:0x%x\n' $addr_base
                printf 'offset:0x%x\n' $((addr_base - orig_addr_base))
                new_to=0x$(printf '%x' $addr_base)
                new_entry=$new_to
                new_iib+=" $element to=$new_to entry=$new_entry"
                addr_base=$((addr_base + file_sz))
                continue
            fi

            echo $element | grep "to=" > /dev/null
            if [[ $? -eq 0 ]];then
                continue
            fi

            echo $element | grep "entry=" > /dev/null
            if [[ $? -eq 0 ]];then
                continue
            fi

            new_iib+=" $element"

        done
        echo "############################################"
        echo "new_iib:"$new_iib
        echo "--------------------------------------------"
        ((i--))
    done
    echo -e "\033[32mfinal:$new_iib \033[37m"
    eval "$ret_ptr=\$new_iib"
}

host_os=`uname -s`
ATB_SIGNER="./sign_tool/linux/atb_signer"
if [ "$host_os" == "Linux" ]; then
ATB_SIGNER="./sign_tool/linux/atb_signer"
else
ATB_SIGNER="./sign_tool/windows/atb_signer"
fi
echo "signer:"$ATB_SIGNER
$ATB_SIGNER  sign --v 2 --sec_ver ${sec_ver} --dgst sha256 \
--iib nohash core=0 image=temp/$DLOADER to=0x404000 entry=0x404000 \
--psn 0x01 --of temp/$DLOADER_SIGNED

ssdk_name=(ssdk_signed_normal.bin ssdk_signed_backup.bin ssdk_signed_third.bin)
ssdk_version=(0x100 0x100 0x100)
BLOCK_SZ=4096

if [ "$enable_reserved" == "true" ];then
    pad_image_to_reserved "$orig_iib_info"
fi

for i in 0 1 2
do

new_iib_info=$orig_iib_info

if [ $XIP_MODE -eq 1 ]; then
    addr_base=$((boot_addr_array[i]+bpt_sz))
    addr_base=$((addr_base+xspi1_mem_base))
    calc_entry "$addr_base" "$orig_iib_info" new_iib_info
fi


$ATB_SIGNER sign --v 2 --sec_ver ${sec_ver} --dgst sha256 $new_iib_info \
--psn ${ssdk_version[$i]} \
--of temp/${ssdk_name[$i]}

FILE_SZ=`stat --format=%s temp/${ssdk_name[$i]}`
MOD=$((FILE_SZ % BLOCK_SZ))
echo "file ${ssdk_name[$i]} sz:"$FILE_SZ
echo "mode:"$MOD

if [ $MOD -gt 0 ];then
    FILE_SZ=$((FILE_SZ + BLOCK_SZ - MOD))
    echo "${ssdk_name[$i]} pad file sz:"$FILE_SZ
    truncate -s $FILE_SZ temp/${ssdk_name[$i]}
fi

echo "Create ${ssdk_name[$i]} done!!"
echo -e "********************************************\n"

done
echo "PYTHON:"${PYTHON}
${PYTHON} pactool/gen_sfs_binary.py --json sfs/${sfs_name} --out temp/sfs.img --normal_img_off ${boot0_addr} --backup_img_off ${boot1_addr} --third_img_off ${boot2_addr}

if [ $PARTITION_MODE -gt 0 ]; then

if [ "$PAC_TYPE" = "ospi" ];then
echo -e " generating PAC with parition : " $OUTPUT_OSPI
${PYTHON} ./pactool/bpttool make_table \
--input bpt/${partition_file} --ab_suffixes "_a,_b" \
--output_json temp/ospi_output.bpt \
--output_gpt temp/ospi_partition.img

${PYTHON} ./pactool/pactool make_pac_image \
--input temp/ospi_output.bpt \
--output $OUTPUT_OSPI \
--image boot0:temp/${ssdk_name[0]} \
--image boot1:temp/${ssdk_name[1]} \
--image boot2:temp/${ssdk_name[2]} \
--image data:temp/data.bin \
--image res:temp/${fat_file} \
--allow_empty_partitions \
--preload sfs:temp/sfs.img \
--preload rfd:rfd/${rfd_name} \
--da FDA:temp/$DLOADER_SIGNED
fi

if [ "$PAC_TYPE" = "emmc" ];then
echo -e " generating PAC with parition : " $OUTPUT_EMMC

${PYTHON} ./pactool/bpttool make_table \
--input bpt/${partition_file} --ab_suffixes "_a,_b" \
--output_json temp/emmc_output.bpt \
--output_gpt temp/emmc_partition.img

${PYTHON} ./pactool/pactool make_pac_image --output $OUTPUT_EMMC --input temp/emmc_output.bpt \
 --image boot2:temp/${ssdk_name[2]} \
 --image data:temp/data.bin \
 --image res:temp/${fat_file} \
 --preload boot0:temp/${ssdk_name[0]} \
 --preload boot1:temp/${ssdk_name[1]} \
 --allow_empty_partitions \
 --da FDA:temp/$DLOADER_SIGNED
fi

else

if [ "$PAC_TYPE" = "ospi" ];then
echo -e " generating PAC  without parition : " $OUTPUT_OSPI
${PYTHON} ./pactool/pactool make_pac_image_no_gpt --output $OUTPUT_OSPI \
--allow_empty_partitions \
--preload sfs:temp/sfs.img \
--preload rfd:rfd/${rfd_name} \
--preload boot0:temp/${ssdk_name[0]} \
--preload boot1:temp/${ssdk_name[1]} \
--preload boot2:temp/${ssdk_name[2]} \
--da FDA:temp/$DLOADER_SIGNED
fi

if [ "$PAC_TYPE" = "emmc" ];then
echo -e " generating PAC without parition : " $OUTPUT_EMMC
${PYTHON} ./pactool/pactool make_pac_image_no_gpt --output $OUTPUT_EMMC \
--allow_empty_partitions \
--preload boot0:temp/${ssdk_name[0]} \
--preload boot1:temp/${ssdk_name[1]} \
--preload boot2:temp/${ssdk_name[2]} \
--da FDA:temp/$DLOADER_SIGNED
fi

fi
