#!/bin/bash

usage()
{
    echo -e "\nUsage: make_vbmeta.sh
    Optional parameters: [-f function] [-d image-description] [-b bpt file] [-s sign parameters]
                         [-o output name] [-t bpttool path] [-i include description]"
echo "
    * [-f function]:          The function to be called,default make_vbmeta.
                              The options are \"make_vbmeta\",\"image_max_size_hash\",\"image_max_size_hashtree\",\"sign_dloader\".
    * [-d image-description]: The description of the images to be signed.
                              This parameter is for make_vbmeta and sign_dloader.
    * [-b bpt file]:          The bpt file containing the images info,such as size,type .
                              This parameter is for make_vbmeta,image_max_size_hash.
    * [-i include description]Included vbmeta description.
                              This parameter is for make_vbmeta.
                              The output vbmeta image includes this description.
    * [-s sign parameters]:   The parameters of signer.
                              This parameter is for make_vbmeta.
    * [-o output name]:       The output file name.
                              This parameter is for make_vbmeta.
    * [-t bpttool path]:      The directory that contains bpttool file.
    * [-p partition_name]:    The partition name in the bpt file.
                              This parameter is for image_max_size_hash and image_max_size_hashtree.
    * [-h]:                   This help message.
"
}

clean_up()
{
    unset make_help make_error args
    unset usage clean_up
}

# get command line options
OLD_OPTIND=$OPTIND

## set default configuration
VBMTEA_DESCIPTOR=
BPT_ORIG_FILE=
VBMETA_IMAGE=
VBMETA_SIGNED_ARGS=
BPTTOOL_DIR=
PARTITION_NAME=
FUNC=make_vbmeta
INCLUDE_DESC=
while getopts "b:d:f:i:o:p:s:t:h" args
do
    case $args in
        f) FUNC="$OPTARG";
           ;;
        d) VBMTEA_DESCIPTOR="$OPTARG";
           ;;
        b) BPT_ORIG_FILE="$OPTARG";
           ;;
        i) INCLUDE_DESC="$OPTARG";
           ;;
        s) VBMETA_SIGNED_ARGS="$OPTARG";
           ;;
        o) VBMETA_IMAGE="$OPTARG";
           ;;
        t) BPTTOOL_DIR="$OPTARG";
           ;;
        p) PARTITION_NAME="$OPTARG";
           ;;
        h) make_help='true';
           ;;
        \?) make_error='true';
           ;;
    esac
done

shift $((OPTIND-1))
if [ $# -ne 0 ]; then
    echo -e "Invalid command line ending: '$@'"
fi
OPTIND=$OLD_OPTIND
if test $make_help; then
    usage && clean_up && exit 1
elif test $make_error; then
    clean_up && exit 1
fi

function image_max_size_hash
{
    res_img_size=$(get_image_max_size "${BPT_ORIG_FILE}" "${PARTITION_NAME}" "hash" )
    res_img_size=`expr ${res_img_size} / 1048576`
    echo "${res_img_size}"
}

function image_max_size_hashtree
{
    res_img_size=$(get_image_max_size "${BPT_ORIG_FILE}" "${PARTITION_NAME}" "hashtree" )
    res_img_size=`expr ${res_img_size} / 1048576`
    echo "${res_img_size}"
}

function make_vbmeta
{
    if [[ x${INCLUDE_DESC} != x ]];then
        INCLUDE_DESC=("${INCLUDE_DESC[@]/#/--include_descriptors_from_image }")
    fi

    VBMTEA_DESCIPTOR=$(images_add_footer ${BPT_ORIG_FILE} "${VBMTEA_DESCIPTOR}")
    VBMTEA_DESCIPTOR+=" ${INCLUDE_DESC}"
    echo "VBMTEA_DESCIPTOR:"${VBMTEA_DESCIPTOR}
    echo "INCLUDE_DESC:"${INCLUDE_DESC}

    make_vbmeta_image "${VBMTEA_DESCIPTOR}" "${VBMETA_SIGNED_ARGS}" "${VBMETA_IMAGE}"
}

function sign_dloader
{
    VBMTEA_DESCIPTOR=(`echo ${VBMTEA_DESCIPTOR}|tr ':' ' '`)
    local IMAGE=${VBMTEA_DESCIPTOR[0]}
    local PARTITION=${VBMTEA_DESCIPTOR[1]}
    local HASH_ALG=${VBMTEA_DESCIPTOR[2]}

    image_add_hash_footer_no_bpt "$IMAGE" "${PARTITION}" "262144" "${HASH_ALG}" "${VBMETA_SIGNED_ARGS}"
}

DIR=$(dirname $0)
export PATH=.:${DIR}:${DIR}/fec:${BPTTOOL_DIR}:${PATH}
export LD_LIBRARY_PATH=.:${DIR}/fec:${LD_LIBRARY_PATH}
source ${DIR}/sign_helper.sh

if [[ x$FUNC == x ]];then
    echo "pls specify function to be called."
    exit -1
fi

$FUNC
exit $?
