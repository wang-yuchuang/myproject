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


./sign_tool/atb_signer  sign --v 2 --sec_ver ${sec_ver} --dgst sha256 \
--iib nohash core=0 image=temp/$DLOADER to=0x404000 entry=0x404000 \
--psn 0x01 --of temp/$DLOADER_SIGNED

ssdk_name=(ssdk_signed_normal.bin ssdk_signed_backup.bin ssdk_signed_third.bin)
ssdk_version=(0x100 0x100 0x100)
BLOCK_SZ=4096


for i in 0 1 2
do

new_iib_info=$orig_iib_info

if [ $XIP_MODE -eq 1 ]; then
    addr_base=$((boot_addr_array[i]+bpt_sz))
    addr_base=$((addr_base+xspi1_mem_base))
    calc_entry "$addr_base" "$orig_iib_info" new_iib_info
fi

./sign_tool/atb_signer sign --v 2 --sec_ver ${sec_ver} --dgst sha256 $new_iib_info \
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

python pactool/gen_sfs_binary.py --json sfs/${sfs_name} --out temp/sfs.img --normal_img_off ${boot0_addr} --backup_img_off ${boot1_addr} --third_img_off ${boot2_addr}


if [ "$PAC_TYPE" = "ospi" ];then
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/${ssdk_name[0]}:boot0:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img"  -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/${ssdk_name[1]}:boot1:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img"  -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/${ssdk_name[2]}:boot2:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img"  -t ./pactool

#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sx1.bin:sx1:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sp0.bin:sp0:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sp1.bin:sp1:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sx0.bin:sx0:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/fat.img:res:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool

echo -e " generating PAC with parition : " $OUTPUT_OSPI

echo -e " generating PAC with parition : " ${partition_file}
echo -e " generating PAC with parition : " ${partition_file}
echo -e " generating PAC with parition : " ${partition_file}
echo -e " generating PAC with parition : " ${partition_file}


./pactool/bpttool make_table \
--input bpt/${partition_file} --ab_suffixes "_a,_b" \
--output_json temp/ospi_output.bpt \
--output_gpt temp/ospi_partition.img

./pactool/pactool make_pac_image \
--input temp/ospi_output.bpt \
--output $OUTPUT_OSPI \
--image boot0:temp/${ssdk_name[0]} \
--image boot1:temp/${ssdk_name[1]} \
--image boot2:temp/${ssdk_name[2]} \
--image sp0:temp/sp0.bin \
--image sp1:temp/sp1.bin \
--image sx0:temp/sx0.bin \
--image sx1:temp/sx1.bin \
--image data:temp/data.bin \
--image res:temp/${fat_file} \
--allow_empty_partitions \
--preload sfs:temp/sfs.img \
--preload rfd:rfd/${rfd_name} \
--da FDA:temp/$DLOADER_SIGNED

else

#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/${ssdk_name[2]}:boot2:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img"  -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sx1.bin:sx1:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sp0.bin:sp0:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sp1.bin:sp1:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/sx0.bin:sx0:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool
#./sign_tool/make_vbmeta.sh -f make_vbmeta -d "temp/fat.img:res:sha256:hash" -b "bpt/${partition_file}" -s "--algorithm SHA256_RSA2048 --key sign_tool/vbmeta/keys/vbmeta-user-key.pem --public_key_meta sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain" -o "temp/vbmeta-ospi.img" -t ./pactool

echo -e " generating PAC with parition : " $OUTPUT_EMMC

./pactool/bpttool make_table \
--input bpt/${partition_file} --ab_suffixes "_a,_b" \
--output_json temp/emmc_output.bpt \
--output_gpt temp/emmc_partition.img

./pactool/pactool make_pac_image --output $OUTPUT_EMMC --input temp/emmc_output.bpt \
 --image boot2:temp/${ssdk_name[2]} \
 --image sp0:temp/sp0.bin \
 --image sp1:temp/sp1.bin \
 --image sx0:temp/sx0.bin \
 --image sx1:temp/sx1.bin \
 --image data:temp/data.bin \
 --image res:temp/${fat_file} \
 --preload boot0:temp/${ssdk_name[0]} \
 --preload boot1:temp/${ssdk_name[1]} \
 --allow_empty_partitions \
 --da FDA:temp/$DLOADER_SIGNED
fi
