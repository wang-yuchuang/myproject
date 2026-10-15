function generate_new_cfg_line()
{
    local partid=$1
    local c=$2
    local membase=$3
    local memsize=$4
    local p=$5

    defconpath=${ROOTDIR}/boards/${BOARD}/${c}/projects/${PROJECT}/defconfig
    `sed -i '/            /d' ${defconpath}`
    echo "CONFIG_${partid}=y            " >> ${defconpath}
    if [ "${p}" != "xip" ];then
        echo "CONFIG_MEM_BASE=${membase}            " >> ${defconpath}
        echo "CONFIG_MEM_SIZE=${memsize}            " >> ${defconpath}
    fi
    echo "CONFIG_RAM_BASE=${membase}            " >> ${defconpath}
    echo "CONFIG_RAM_SIZE=${memsize}            " >> ${defconpath}
}
function conver_val_to_str()
{
    local str=$1

    result=`echo ${str} | awk '{printf "0x%x\n",$0}'`

    echo $result
    return $?
}
function generate_chipid_deconfig()
{
    local partid=$1
    local prjname=$2
    local partpath=${ROOTDIR}/arch/arm/e3/part
    local corenum=1
    local sp_available=0
    local sx_available=0
    if [ $(is_taishan_platform ${BOARD}) == 1 ]; then
        if [ -e ${partpath}/${partid^^}.Kconfig ];then

            if [[ `grep -w -A 3 "^config PART_SP" ${partpath}/${partid^^}.Kconfig | head -3` == *default*y* ]];then
                corenum=`expr $corenum + 2`
                sp_available=1
            fi

            if [[ `grep -w -A 3 "^config PART_SX" ${partpath}/${partid^^}.Kconfig | head -3` == *default*y* ]];then
                corenum=`expr $corenum + 2`
                sx_available=1
            fi

            echo ${partid^^} "core number is " ${corenum}

            IRAM_STR_RAW=`grep -w -A 3 "config IRAM_LIMIT" ${partpath}/${partid^^}.Kconfig | head -3`
            IRAM_STR=$(echo ${IRAM_STR_RAW} | grep -Eo 0x[0-9a-z]* | head -1)
            IRAM_LIMIT_VALUE=$((16#${IRAM_STR#"0x"}))

            echo ${partid^^} "iram size is " `conver_val_to_str ${IRAM_LIMIT_VALUE}`

            local SF_ALLOCATE_MEM_BASE=4210688

            SF_ALLOCATE_MEM_BASE_STR=`conver_val_to_str ${SF_ALLOCATE_MEM_BASE}`
            echo "Allocate SF iram base is" ${SF_ALLOCATE_MEM_BASE_STR}

            OTHER_CORE_NUM=`expr ${corenum} - 1`
            OTHER_CORE_ALLOCATE_MEM_SIZE=`expr ${OTHER_CORE_NUM} \* 393216`

            if [[ $IRAM_LIMIT_VALUE -lt ` expr ${OTHER_CORE_ALLOCATE_MEM_SIZE} + 16384 ` ]];then
                echo "iram too small error"
                exit
            fi
            SF_ALLOCATE_MEM_SIZE=`expr $IRAM_LIMIT_VALUE - ${OTHER_CORE_ALLOCATE_MEM_SIZE} - 16384`
            SF_ALLOCATE_MEM_SIZE_STR=`conver_val_to_str ${SF_ALLOCATE_MEM_SIZE}`
            echo "Allocate SF iram size is" ${SF_ALLOCATE_MEM_SIZE_STR}
            generate_new_cfg_line ${partid^^} "sf" ${SF_ALLOCATE_MEM_BASE_STR} ${SF_ALLOCATE_MEM_SIZE_STR} ${prjname}

            if [ ${sp_available} == 1 ];then
                SP0_ALLOCATE_MEM_BASE=`expr $SF_ALLOCATE_MEM_BASE + $SF_ALLOCATE_MEM_SIZE`
                SP0_ALLOCATE_MEM_SIZE=393216

                SP0_ALLOCATE_MEM_BASE_STR=`conver_val_to_str ${SP0_ALLOCATE_MEM_BASE}`
                echo "Allocate SP0 iram base is" ${SP0_ALLOCATE_MEM_BASE_STR}
                SP0_ALLOCATE_MEM_SIZE_STR=`conver_val_to_str ${SP0_ALLOCATE_MEM_SIZE}`
                echo "Allocate SP0 iram size is" ${SP0_ALLOCATE_MEM_SIZE_STR}

                generate_new_cfg_line ${partid^^} "sp0" ${SP0_ALLOCATE_MEM_BASE_STR} ${SP0_ALLOCATE_MEM_SIZE_STR} ${prjname}

                SP1_ALLOCATE_MEM_BASE=`expr $SP0_ALLOCATE_MEM_BASE + $SP0_ALLOCATE_MEM_SIZE`
                SP1_ALLOCATE_MEM_SIZE=393216

                SP1_ALLOCATE_MEM_BASE_STR=`conver_val_to_str ${SP1_ALLOCATE_MEM_BASE}`
                echo "Allocate SP1 iram base is" ${SP1_ALLOCATE_MEM_BASE_STR}
                SP1_ALLOCATE_MEM_SIZE_STR=`conver_val_to_str ${SP1_ALLOCATE_MEM_SIZE}`
                echo "Allocate SP1 iram size is" ${SP1_ALLOCATE_MEM_SIZE_STR}

                generate_new_cfg_line ${partid^^} "sp1" ${SP1_ALLOCATE_MEM_BASE_STR} ${SP1_ALLOCATE_MEM_SIZE_STR} ${prjname}

            fi
            if [ ${sx_available} == 1 ];then
                if [ ${sp_available} == 1 ];then
                    SX0_ALLOCATE_MEM_BASE=`expr $SP1_ALLOCATE_MEM_BASE + $SP1_ALLOCATE_MEM_SIZE`
                else
                    SX0_ALLOCATE_MEM_BASE=`expr $SF_ALLOCATE_MEM_BASE + $SF_ALLOCATE_MEM_SIZE`
                fi
                SX0_ALLOCATE_MEM_SIZE=393216

                SX0_ALLOCATE_MEM_BASE_STR=`echo ${SX0_ALLOCATE_MEM_BASE} | awk '{printf "0x%x\n",$0}'`
                echo "Allocate SX0 iram base is" ${SX0_ALLOCATE_MEM_BASE_STR}
                SX0_ALLOCATE_MEM_SIZE_STR=`echo ${SX0_ALLOCATE_MEM_SIZE} | awk '{printf "0x%x\n",$0}'`
                echo "Allocate SX0 iram size is" ${SX0_ALLOCATE_MEM_SIZE_STR}

                generate_new_cfg_line ${partid^^} "sx0" ${SX0_ALLOCATE_MEM_BASE_STR} ${SX0_ALLOCATE_MEM_SIZE_STR} ${prjname}

                SX1_ALLOCATE_MEM_BASE=`expr $SX0_ALLOCATE_MEM_BASE + $SX0_ALLOCATE_MEM_SIZE`
                SX1_ALLOCATE_MEM_SIZE=393216

                SX1_ALLOCATE_MEM_BASE_STR=`echo ${SX1_ALLOCATE_MEM_BASE} | awk '{printf "0x%x\n",$0}'`
                echo "Allocate SX1 iram base is" ${SX1_ALLOCATE_MEM_BASE_STR}
                SX1_ALLOCATE_MEM_SIZE_STR=`echo ${SX1_ALLOCATE_MEM_SIZE} | awk '{printf "0x%x\n",$0}'`
                echo "Allocate SX1 iram size is" ${SX1_ALLOCATE_MEM_SIZE_STR}

                generate_new_cfg_line ${partid^^} "sx1" ${SX1_ALLOCATE_MEM_BASE_STR} ${SX1_ALLOCATE_MEM_SIZE_STR} ${prjname}
                echo "auto id configuration generate success"
            fi
        else
            echo "${chip_id} is invaild chip id , auto id configuration function failed"
        fi
    fi
}

chip_id=$1
prj_name=$2
generate_chipid_deconfig ${chip_id} ${prj_name}
echo "auto id configuration generate success"