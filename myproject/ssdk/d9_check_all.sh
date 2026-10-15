#!/bin/bash
CRTDIR=$(pwd)

echo "running in ${CRTDIR}"

if [ ! -d $CRTDIR/boards/ ];then
  echo -e "\\033[31mOpps,the board dir is not exist !! \033[0m"
  exit 1
fi

IGNORE_BOARDS=(
    d3_disp
    d3_gateway
    e3_144_evb
    e3_144_ref
    e3_176_evb
    e3_176_ref
    e3_disp
    e3_evb
    e3_gateway
    e3_motor
    g9x_ref
    taishan_haps
    taishan_ll_z1
    taishan_l_z1
    taishan_tianjie
    taishan_z1
    x9hp_ms
    x9h_ref
    x9hp_ms_x9sp_ea
    x9hp_ms_a12_vemmc
    x9sp_ms
    x9cc_a
    x9cc_b
    x9cc_ms_a
    x9cc_ms_b
    x9sp_ms_demo
    x9u-ref_a
    x9u-ref_b
    v9p_ms
    x9hp_ms_v9p_ea
    x9sp_evb
)

IGNORE_PROJECTS=(
    g001
    h001
    h002
)

function replace_search_one_then_add_line() {
    f_dir=$1
    f_name=$2
    src_string=$3
    dst_string=$4
    echo "replace_search_one_then_add_line: search $1 dir for keyword:$2. then add $4 in new line"
    for choose_file in `find ${f_dir} -name ${f_name}`; do
        echo "$choose_file";
        L=`sed -n "/${src_string}/=" $choose_file | head -1`;
        if [ "x$L" != "x" ];then
            sed -i "${L}a ${dst_string}" $choose_file
        fi
    done
}

function replace_del_all_match_line() {
    f_dir=$1
    f_name=$2
    src_string=$3
    echo "replace_del_all_match_line: search $1 dir for keyword:$2.then del all match line"
    for choose_file in `find ${f_dir} -name ${f_name}`; do
        echo "$choose_file";
        for line in `sed -n "/${src_string}/=" $choose_file | sort -r`; do
            sed -i "${line}d" $choose_file;
        done
    done
}

case_num=`echo "0 1 2 3 4 5"`
defconfig_add_str=""

function do_some_change()
{
    board=$1
    cnt=$2
    echo "board=$board cnt=$cnt"
    case "$cnt" in
    0)
        defconfig_add_str="CONFIG_DWCMSHC CONFIG_KUNLUN_MMC CONFIG_MMC_TESTER"
        if [ -e boards/${board}/safety ];then
            for str in $defconfig_add_str;do
                replace_search_one_then_add_line "boards/${board}/safety" "defconfig" "CONFIG_MEM_BASE" "$str=y"
            done
        fi
        ;;
    1)
        defconfig_add_str="CONFIG_DMA_TESTER CONFIG_DW_DMA"
        for core in `echo "safety secure"`;do
            if [ ${board} == "d9310_ref" ] && [ "$core" == "secure" ];then echo "${board} secure dont test dma";continue;fi
            if [ ${board} == "d9342_ref" ] && [ "$core" == "secure" ];then echo "${board} secure dont test dma";continue;fi
            if [ ${board} == "d9360_ref" ] && [ "$core" == "secure" ];then echo "${board} secure dont test dma";continue;fi
            if [ -e boards/${board}/$core ];then
                for str in $defconfig_add_str;do
                replace_search_one_then_add_line "boards/${board}/$core" "defconfig" "CONFIG_MEM_BASE" "$str=y"
                done
            fi
        done
        ;;
    2)
        defconfig_add_str="CONFIG_DW_I2C CONFIG_I2CTOOLS CONFIG_I2C_TESTER"
        for core in `echo "safety secure"`;do
            if [ -e boards/${board}/$core ];then
                for str in $defconfig_add_str;do
                replace_search_one_then_add_line "boards/${board}/$core" "defconfig" "CONFIG_MEM_BASE" "$str=y"
                done
            fi
        done
        ;;
    3)
        if [ -e boards/${board}/safety ];then
        replace_search_one_then_add_line "boards/${board}/safety" "defconfig" "CONFIG_MEM_BASE" "CONFIG_VERIFIED_BOOT=y"
        defconfig_add_str="CONFIG_VERIFIED_BOOT=y"
        fi
        ;;
    4)
        defconfig_add_str="CONFIG_EARLYCOPY CONFIG_DW_DMA CONFIG_PFM CONFIG_FUSALIB_DDR CONFIG_APPLICATION CONFIG_FuncSafetyPkg_TEST"
        if [ -e boards/${board}/safety ];then
            for str in $defconfig_add_str;do
                replace_search_one_then_add_line "boards/${board}/safety" "defconfig" "CONFIG_MEM_BASE" "$str=y"
            done
        fi
        ;;
    5)
        defconfig_add_str="CONFIG_OS_BAREMETAL=y"
        if [ -e boards/${board}/safety ];then
        replace_search_one_then_add_line "boards/${board}/safety" "defconfig" "CONFIG_MEM_BASE" "CONFIG_OS_BAREMETAL=y"
        fi
        ;;
    esac
}

BOARDS=$(ls -l $CRTDIR/boards/ |awk '/^d/ {print $NF}')
for board in $BOARDS
do
    if [[ $(echo $board | grep "archived") == "" ]]; then
        if [[ ! "${IGNORE_BOARDS[@]}"  =~ "$board" ]]; then
            if [ -d $CRTDIR/boards/$board/build ];then rm -rf $CRTDIR/boards/$board/build;fi
            PROJECTS=$(find $CRTDIR/boards/$board -name projects -type d | xargs ls -l |awk '/^d/ {print $NF}')
            for project in $PROJECTS
            do
                if [[ ! "${IGNORE_PROJECTS[@]}"  =~ "$project" ]]; then
                    for count in ${case_num}
                    do
                        do_some_change ${board} $count
                        echo "./build.sh -b $board -p $project -clear"
                        ./build.sh -b $board -p $project -clear
                        if [ $? -ne 0 ]; then
                            echo -e "\\033[31mOpps, ./build.sh -b $board -p $project faild !!! defconfig_add:$defconfig_add_str\033[0m"
                            exit 1
                        fi
                        git checkout boards;
                    done
                fi
            done
        fi
    fi
done

echo -e "\\033[32mAll build d9 item pass.\n\n \033[0m"
exit 0
