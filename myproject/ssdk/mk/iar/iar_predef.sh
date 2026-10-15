#!/bin/bash

############################################################################
# VARIABLES
############################################################################

ALLSRCFILE="$1";shift
TOP="$1";shift
CFLAGS_DEFS="$1";shift
CFLAGS_INC="$1";shift
LDFLAGS="$1";shift
IAR_PROG_ENTRY="$1";shift
IAR_LINKER="$1";shift
IAR_LINKER_OPTIONS="$1";shift
IAR_DEBUGGER="$1";shift
IAR_PRJ_DIR="$1";shift

fRenamedSrc=""
fAsmSrc=""

SrcGroupNames=()

############################################################################
# FUNCTIONS
############################################################################

getAsmSrc(){
    local parasrc="$1"
    for src in ${parasrc}; do
        postfix=${src##*.}
        if [ ${postfix} == "S" ]; then
            #src=$(dirname $src)"/toolchain/iar/"$(basename $src)
            src=$(dirname $src)"/"$(basename $src)
        fi
        fAsmSrc="$fAsmSrc $src"
    done
}

# duplicate file in different folder ,failed to generate *pbi in same obj folder
renamesrc(){
    local tmpsrc
    local tmpfile
    local result
    local alla
    local parasrc="$1"
    for src in ${parasrc}; do
        tmpsrc=$(basename $src)
        # check for filename collision (even with different file suffixes)
        # e.g. file.S collides with file.c after being compiled to file.o
        if [[ $tmpfile =~ ${tmpsrc%%.*} ]]; then
            arr=($tmpfile)
            for ele in ${arr[*]}; do
                if [ ${tmpsrc%%.*} == ${ele%%.*} ]; then
                #if [ ${#tmpsrc} == ${#ele} ]; then
                    dirsrc=$(dirname $src)
                    lstdir=${dirsrc##*/}
                    oldsrc=$tmpsrc
                    tmpsrc=$lstdir"_"$tmpsrc
                    src=$dirsrc"/"$tmpsrc
                    if [ -f $src ]; then
                        rm $src
                    fi
                    touch $src
                    echo \#include \"$oldsrc\" >> $src
                fi
            done
        fi
        tmpfile="$tmpfile $tmpsrc"
        fRenamedSrc="$fRenamedSrc $src"
    done
}

parsesrc() {
    local pos
    local tmpsrc
    local para="$1"
    for src in ${para}; do
        pos=`expr index $src /`
        tmpsrc=${src:0:pos-1}
        # get the group name of the current source
        SrcGroupNames+=("$tmpsrc")
        # place together the sources in the same group
        SrcsInEachGroup="sources_in_${tmpsrc}"
        printf -v "$SrcsInEachGroup" '%s' "${!SrcsInEachGroup} $src"
    done

    # add usrlibs
    handleLib "$LDFLAGS"
    SrcGroupNames+=("usrlibs")
    SrcsInEachGroup="sources_in_usrlibs"
    printf -v "$SrcsInEachGroup" '%s' "${!SrcsInEachGroup} $lib_y"

    # remove the duplicate group name
    SrcGroupNames=($(echo ${SrcGroupNames[@]} | tr [:space:] '\n' | awk '!a[$0]++'))
}

handleLib(){
    local flag="$1"
    local path
    local lib

    for item in ${flag}; do
        if [[ ${item} == -L* ]]; then
            path=${item:2}
        elif [[ ${item} == -l* ]]; then
            lib=$path"/lib"${item:2}".a"
            lib_y="$lib_y $lib"
        fi
    done
}

create(){
    echo GEN Environment $IAR_PRJ_DIR/.env_gen.sh
    touch $IAR_PRJ_DIR/.env_gen.sh
    echo TOP=\"$TOP\"                                       >> $IAR_PRJ_DIR/.env_gen.sh
    echo CFLAGS_DEFS=\'$CFLAGS_DEFS\'                       >> $IAR_PRJ_DIR/.env_gen.sh
    echo CFLAGS_INC=\"$CFLAGS_INC\"                         >> $IAR_PRJ_DIR/.env_gen.sh
    echo iar_linker_options_y=\"$IAR_LINKER_OPTIONS\"       >> $IAR_PRJ_DIR/.env_gen.sh
    echo iar_linker_script_y=\"$IAR_LINKER\"                >> $IAR_PRJ_DIR/.env_gen.sh
    echo iar_debug_script_y=\"$IAR_DEBUGGER\"               >> $IAR_PRJ_DIR/.env_gen.sh
    echo iar_program_entry_y=\"$IAR_PROG_ENTRY\"            >> $IAR_PRJ_DIR/.env_gen.sh

    # pass the name and sources of every group to .env_gen.sh
    echo SRC_GROUP=\"${SrcGroupNames[@]}\"                  >> $IAR_PRJ_DIR/.env_gen.sh
    for group in "${SrcGroupNames[@]}"; do
        SrcsInEachGroup="sources_in_${group}"
        echo ${group}_y=\"${!SrcsInEachGroup}\"             >> $IAR_PRJ_DIR/.env_gen.sh
    done
}

############################################################################
# SCRIPT
############################################################################

#Replace GNU ASM with ARM ASM
getAsmSrc "$ALLSRCFILE"

#Rename duplicated file
renamesrc "$fAsmSrc"
parsesrc "$fRenamedSrc"
create
