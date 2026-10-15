#!/bin/bash

export BOARD
export PROJECT
export CORE

usage()
{
    echo -e "\nUsage: menuconfig.sh
    Optional parameters: [-b board] [-p project] [-c core]"
echo "
    * [-b board]:            board name
    * [-p project]:          project name
    * [-c core]:             core name
    * [-h]:                  This help message
"
}

clean_up()
{
    unset flags help error
    unset usage clean_up
}

# get command line options
OLD_OPTIND=$OPTIND

while getopts "b:p:c:h" flags
do
    case $flags in
        b) BOARD="$OPTARG";
           ;;
        p) PROJECT="$OPTARG";
           ;;
        c) CORE="$OPTARG";
           ;;
        h) help='true';
           ;;
        \?) error='true';
           ;;
    esac
done

shift $((OPTIND-1))
if [ $# -ne 0 ]; then
    echo -e "Invalid command line ending: '$@'"
fi
OPTIND=$OLD_OPTIND
if test $help; then
    usage && clean_up && exit 1
elif test $error; then
    clean_up && exit 1
fi

echo -e " Config project  : " $PROJECT

SSDKROOT=${PWD}
POJECT_PATH=$SSDKROOT/boards/$BOARD/$CORE/projects/$PROJECT

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PYTHON=python3
elif [[ "$OSTYPE" == "msys" ]]; then
    PYTHON=$SSDKROOT/prebuilts/windows/python-3.7.0/python.exe
else
    echo "Unknown OSTYPE $OSTYPE"
    exit 1
fi

$PYTHON $SSDKROOT/tools/SSDKDS/scripts/menuconfig.py --kconfig_path=$SSDKROOT \
        --defconfig_path=$POJECT_PATH --out_path=$POJECT_PATH