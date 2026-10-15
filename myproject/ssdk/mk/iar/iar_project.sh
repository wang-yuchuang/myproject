#!/bin/bash
# Generate IAR projects

set -e

############################################################################
# INPUTS
############################################################################

DIR="$1";shift
TOP="$1";shift
PRJ_ROOT="$1";shift
BOARD_NAME="$1";shift
PRJ_NAME="$1";shift
CORE_NAME="$1";shift
WORKSPACE_TEMPLATE="$1";shift
PROJECT_TEMPLATE="$1";shift
DEBUG_TEMPLATE="$1";shift
IAR_PRJ_DIR="$1";shift
IAR_LINK_DIR="$1";shift

############################################################################
# FUNCTIONS
############################################################################

tpl-split() {
    local template_src="$1"
    local template="$2"
    local nod="$3"
    local head
    local tail

    echo "SPLIT $template_src"

    head=$(sed -n "/<${nod}>/=" "${template_src}" | sed -n '1p')
    tail=$(sed -n "/<\/${nod}>/=" "${template_src}" | sed -n '$p')

    # head: from start of file to <configuration> (excluded)
    sed -n "1,`expr ${head} - 1`p" "${template_src}" > "$IAR_PRJ_DIR/${template}.head"
    # body: from <configuration> to </configuration> (both included)
    sed -n "${head},${tail}p" "${template_src}" > "$IAR_PRJ_DIR/${template}.body"
    # tail: from </configuration> (excluded) to end of file
    sed -n "`expr ${tail} + 1`,\$p" "${template_src}" > "$IAR_PRJ_DIR/${template}.tail"
}

helper-use-windows-path() {
    local str=${1//\/\//\/}
    echo ${str//\//\\\\}
}

find-source() {
    local dir="$1"
    local obj="$2"
    local path=${obj//.o/.c}
    if [ ! -f "$dir$path" ]; then
        path=${obj//.o/.s}

        if [ ! -f "$dir/$path" ]; then

            echo "Source file for $obj not found!" 1>&2
            exit 3
        fi
    fi

}

tpl-set-prj-files() {
    local tpl="$1"
    local section="$2"; shift; shift
    local input="$*"
    local temp

    local tmpxml=`mktemp -p "$IAR_PRJ_DIR"`
    (
        for path in ${input}; do
            temp=$(find-source "$DIR" "$path")
            local win_path=$(helper-use-windows-path "$path")
            echo -e "  <file><name>${IAR_TOP_RELATIVE}\\\\${win_path}</name></file>"
        done
    ) > "$tmpxml"

    sed -i -e "/__REPLACE_PROJECT_FILES__/r $tmpxml" "$tpl"
    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-deps() {
    local tpl="$1"
    local section="$2"; shift; shift

    local input=$*
    local empty_group=true
    local tmpxml=`mktemp -p "$IAR_PRJ_DIR"`
    local temp

    # Read file and fill input array
    (

        for path in ${input}; do
            temp=$(find-source "$DIR" "$path")

            if [ "$empty_group" = true ]
            then
                empty_group=false
                echo -e "  <group>\n    <name>$section</name>"
            fi

            local win_path=$(helper-use-windows-path "$path")
            echo -e "    <file><name>${IAR_TOP_RELATIVE}\\\\${win_path}</name></file>"
        done

        if [ "$empty_group" = false ]
        then
            echo -e "  </group>"
        fi
    ) > "$tmpxml"

    sed -i -e "/__REPLACE_DEP_LIST__/r $tmpxml" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-version() {
    local tpl="$1"
    sed -i -e "s/__REPLACE_VERSION__/$VERSION/g" "$tpl"
}

tpl-finalize() {
    local tpl="$1"

    sed -i "s/\(__REPLACE_DEP_LIST__\|__REPLACE_PROJECT_FILES__\)//g" "$tpl"
}

tpl-set-defines() {
    local tpl="$1"
    local tmpxml=`mktemp -p "$IAR_PRJ_DIR"`

    for flag in $CFLAGS_DEFS; do
        flag=${flag//:/ }
        echo "          <state>${flag//-D/}</state>"
    done > "$tmpxml"

    sed -i -e "/__REPLACE_DEFINES__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_DEFINES__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-includes() {
    local tpl="$1"
    local tmpxml=`mktemp -p "$IAR_PRJ_DIR"`

    (
        for include in $CFLAGS_INC; do
            local inc=$(helper-use-windows-path "${include//-I/}")
            echo -e "          <state>${IAR_TOP_RELATIVE}\\\\${inc}</state>"
        done
        echo -e "          <state>${IAR_PRJ_RELATIVE}</state>"
        echo -e "          <state>\$PROJ_DIR\$</state>"
    ) > "$tmpxml"

    sed -i -e "/__REPLACE_INCLUDES__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_INCLUDES__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-preincludes() {
    local tpl="$1"
    local tmpxml=`mktemp -p "$IAR_PRJ_DIR"`

    echo -e "          <state>${IAR_PRJ_RELATIVE}\\\\config.h</state>" > "$tmpxml"

    sed -i -e "/__REPLACE_PREINCLUDES__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_PREINCLUDES__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-linker-options() {
    local tpl="$1"
    local linker_options="$2"
    local tmpxml=`mktemp -p "$IAR_PRJ_DIR"`

    for symbol in $linker_options; do
        echo "          <state>${symbol}</state>"
    done > "$tmpxml"

    sed -i -e "/__REPLACE_LINKER_OPTIONS__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_LINKER_OPTIONS__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-linker-script() {
    local tpl="$1"
    local linker_script="$2"
    if [ ! -f $linker_script ]; then
        echo "File $linker_script not found!" 1>&2
        exit 3
    fi

    sed -i -e "s%__REPLACE_LINK_SCRIPT__%$IAR_LINK_RELATIVE\\\\$(basename $linker_script)%g" "$tpl"
}

tpl-set-program-entry() {
    local tpl="$1"
    local entry="$2"

    sed -i -e "s%__REPLACE_ENTRY__%$entry%g" "$tpl"
}

tpl-set-binary-name() {
    local tpl="$1"
    local binary_name="$2"

    if [ -z $binary_name ]; then
        echo binary_name not defined! 1>&2
        exit 3
    fi

    sed -i -e "s/__REPLACE_BIN_NAME__/$binary_name/g" "$tpl"
}

tpl-set-configuration() {
    local tpl="$1"
    local target=$2

    echo "SET target=$PRJ_NAME"
    sed -i "s/__REPLACE_TARGET__/$PRJ_NAME/g" "$tpl"
}

tpl-set-chip() {
    local tpl="$1"
    local chip=

    for flag in $CFLAGS_DEFS; do
        if [ $flag == "ARM_CPU_cortex-r5=1" ]; then
            chip="Cortex-R5"
        fi
    done

    echo "SET CHIP=$chip"
    sed -i "s/__REPLACE_CHIP__/$chip/g" "$tpl"
}

tpl-set-prebuild() {
    local tpl="$1"

    sed -i "s/__REPLACE_PREBUILD__/$IAR_TOP_RELATIVE\\\\tools\\\\iarprebuild.bat $BOARD_NAME $PRJ_NAME $CORE_NAME $IAR_TOP_RELATIVE/g" "$tpl"
}

tpl-set-postbuild() {
    local tpl="$1"

    sed -i "s/__REPLACE_POSTBUILD__/$IAR_TOP_RELATIVE\\\\tools\\\\sdtools\\\\package.bat $BOARD_NAME $PRJ_NAME/g" "$tpl"
}

generate-bodies-ewd() {
    local file="$1"
    local tpl="$IAR_PRJ_DIR/${file}.ewd"
    local board_file

    echo "GEN temporary file ${file}.ewd"

    cat "$IAR_PRJ_DIR/iar_debug.body" > "$tpl"
    sed -i -e "s%__REPLACE_MACFILE__%$IAR_LINK_RELATIVE\\\\$(basename $iar_debug_script_y)%g" "$tpl"
    sed -i -e "s%//%/%g" "$tpl"

    if [ -e "$IAR_LINK_DIR/flashloader" ]; then
        sed -i -e "s%__REPLACE_FLASH_LOADER__%1%g" "$tpl"
        if [[ "$PRJ_NAME" == xip ]]; then
            board_file="iar_flashboardcfg_xip.board"
        else
            board_file="iar_flashboardcfg.board"
        fi
        sed -i -e "s%__REPLACE_FLASH_LOADER_V3__%$IAR_LINK_RELATIVE\\\\flashloader\\\\$board_file%g" "$tpl"
    fi

    if [[ "$BOARD_NAME" == e3* ]]; then
        sed -i -e "s%__REPLACE_EXTRA_OPTION_CHK__%1%g" "$tpl"
        sed -i -e "s%__REPLACE_JLINK_SCRIPT__%--jlink_script_file=$IAR_TOP_RELATIVE\\\\tools\\\\JLINK\\\\e3\\\\$CORE_NAME.JLinkScript%g" "$tpl"
        sed -i -e "s%__REPLACE_JLINK_INTERFACE__%1%g" "$tpl"
    elif [[ "$BOARD_NAME" == d3* ]]; then
        sed -i -e "s%__REPLACE_EXTRA_OPTION_CHK__%1%g" "$tpl"
        sed -i -e "s%__REPLACE_JLINK_SCRIPT__%--jlink_script_file=$IAR_TOP_RELATIVE\\\\tools\\\\JLINK\\\\d3\\\\$CORE_NAME.JLinkScript%g" "$tpl"
        sed -i -e "s%__REPLACE_JLINK_INTERFACE__%1%g" "$tpl"
    else
        sed -i -e "s%__REPLACE_EXTRA_OPTION_CHK__%0%g" "$tpl"
        sed -i -e "s%__REPLACE_JLINK_SCRIPT__%%g" "$tpl"
        sed -i -e "s%__REPLACE_JLINK_INTERFACE__%0%g" "$tpl"
    fi

    tpl-set-configuration "$tpl" $CORE_NAME
}

generate-ewd() {
    local file="$1"
    local tpl="$IAR_PRJ_DIR/${file}.ewd"
    if [ -f "$tpl" ]; then
        rm -f $tpl
    fi

    tpl-split "$DEBUG_TEMPLATE" iar_debug configuration

    rm -f "$IAR_PRJ_DIR/$file.ewd.bodies"
    if [ -f "$IAR_PRJ_DIR/.env_gen.sh"  ]; then
        . "$IAR_PRJ_DIR/.env_gen.sh"
        generate-bodies-ewd "$file"
    fi

    # Remove temprary files
    rm -f "$IAR_PRJ_DIR/$file.ewd.bodies"
    touch "$IAR_PRJ_DIR/$file.ewd.bodies"
    cat "$IAR_PRJ_DIR/${file}.ewd" >> "$IAR_PRJ_DIR/$file.ewd.bodies"
    rm -f "$IAR_PRJ_DIR/${file}.ewd"

    tpl-set-chip "$IAR_PRJ_DIR/$file.ewd.bodies"

    echo "GEN ${file}.ewd"
    cat "$IAR_PRJ_DIR/iar_debug.head" > "$tpl"
    rm -f "$IAR_PRJ_DIR/iar_debug.head"
    cat "$IAR_PRJ_DIR/$file.ewd.bodies" >> "$tpl"
    rm -f "iar_debug.body"
    rm -f "$IAR_PRJ_DIR/$file.ewd.bodies"
    cat "$IAR_PRJ_DIR/iar_debug.tail" >> "$tpl"
    rm -f "$IAR_PRJ_DIR/iar_debug.tail"
}

generate-bodies-ewp() {
    local file="$1"
    local tpl="$IAR_PRJ_DIR/${file}.ewp"

    echo "GEN temporary file ${file}.ewp"
    cat "$IAR_PRJ_DIR/iar_project.body" > "$tpl"
    tpl-set-defines         "$tpl"
    tpl-set-includes        "$tpl"
    tpl-set-preincludes     "$tpl"
    tpl-set-linker-options  "$tpl" "$iar_linker_options_y"
    tpl-set-linker-script   "$tpl" "$iar_linker_script_y"
    tpl-set-program-entry   "$tpl" "$iar_program_entry_y"
    tpl-set-binary-name     "$tpl" "$file"
    tpl-set-configuration   "$tpl" "$CORE_NAME"
    tpl-set-prebuild        "$tpl"
    tpl-set-postbuild       "$tpl"
}

generate-ewp() {
    local file="$1"
    local tpl="$IAR_PRJ_DIR/${file}.ewp"
    if [ -f ${tpl} ]; then
        rm -f ${tpl}
    fi
    tpl-split "$PROJECT_TEMPLATE" iar_project configuration

    if [ -f "$IAR_PRJ_DIR/.env_gen.sh"  ]; then
        . "$IAR_PRJ_DIR/.env_gen.sh"
        generate-bodies-ewp "$file"
    fi

    # Remove temporary files after merging them
    rm -f "$IAR_PRJ_DIR/$file.ewp.bodies"
    touch "$IAR_PRJ_DIR/$file.ewp.bodies"

    cat "$IAR_PRJ_DIR/${file}.ewp" >> "$IAR_PRJ_DIR/$file.ewp.bodies"
    rm -f "$IAR_PRJ_DIR/${file}.ewp"

    echo "GEN ${file}.ewp"
    cat "$IAR_PRJ_DIR/iar_project.head" >> "$tpl"
    rm -f "$IAR_PRJ_DIR/iar_project.head"
    cat "$IAR_PRJ_DIR/$file.ewp.bodies" >> "$tpl"
    rm -f "$IAR_PRJ_DIR/iar_project.body"
    rm -f "$IAR_PRJ_DIR/$file.ewp.bodies"
    cat "$IAR_PRJ_DIR/iar_project.tail" >> "$tpl"
    rm -f "$IAR_PRJ_DIR/iar_project.tail"

    # fill the template with the sources in each group
    for group in $SRC_GROUP; do
        source_y="${group}_y"
        tpl-set-deps "$tpl" "$group" "${!source_y}"
    done

    tpl-set-chip      "$tpl"
    #tpl-set-prj-files "$tpl" "project_files" "$top_y"
    tpl-finalize      "$tpl"
}

generate-bodies-eww() {
    local file="$1"
    local tpl="$IAR_PRJ_DIR/iar_workspace.eww"

    echo "GEN temporary file iar_workspace.eww"
    cat "$file" > "$tpl"
}

generate-eww() {
    local file=$1
    local tpl="$IAR_PRJ_DIR/${file}.eww"
    if [ -f $tpl ]; then
        rm -f $tpl
    fi
    if [ -f "$IAR_PRJ_DIR/${file}.eww" ]; then
       echo "UPD ${file}.eww"
       tpl-split "$WORKSPACE_TEMPLATE" iar_workspace project
       generate-bodies-eww "$IAR_PRJ_DIR/iar_workspace.body"
       rm -f "$IAR_PRJ_DIR/iar_workspace.head"
       rm -f "$IAR_PRJ_DIR/iar_workspace.body"
       rm -f "$IAR_PRJ_DIR/iar_workspace.tail"

       tpl-split "$tpl" iar_workspace project
       cat "$IAR_PRJ_DIR/iar_workspace.head" > "$tpl"
       rm -f "$IAR_PRJ_DIR/iar_workspace.head"
       cat "$IAR_PRJ_DIR/iar_workspace.body" >> "$tpl"
       sed -i "s%__REPLACE_WORKSPACE_FILE__%\$WS_DIR\$/${file}.ewp%g" "$IAR_PRJ_DIR/iar_workspace.eww"
       cat "$IAR_PRJ_DIR/iar_workspace.eww" >> "$tpl"
       rm -f "$IAR_PRJ_DIR/iar_workspace.body"
       rm -f "$IAR_PRJ_DIR/iar_workspace.eww"
       cat "$IAR_PRJ_DIR/iar_workspace.tail" >> "$tpl"
       rm -f "$IAR_PRJ_DIR/iar_workspace.tail"

    else
       echo "GEN ${file}.eww"
       sed -e "s%__REPLACE_WORKSPACE_FILE__%\$WS_DIR\$/${file}.ewp%g" < "$WORKSPACE_TEMPLATE" |
       sed -e "s%//%/%g" > "$IAR_PRJ_DIR/${file}.eww"
    fi
}

############################################################################
# SCRIPT VARS
############################################################################

# relative path from $IAR_PRJ_DIR to $TOP
TOP_RELATIVE="$(realpath --relative-to="$IAR_PRJ_DIR" "$TOP")"
# relative path from $IAR_PRJ_DIR to $PRJ_ROOT
PRJ_RELATIVE="$(realpath --relative-to="$IAR_PRJ_DIR" "$PRJ_ROOT")"
# relative path from $IAR_PRJ_DIR to $IAR_LINK_DIR
LINK_RELATIVE="$(realpath --relative-to="$IAR_PRJ_DIR" "$IAR_LINK_DIR")"

# top relative path in IAR-accepted format
IAR_TOP_RELATIVE="\$PROJ_DIR\$\\\\$(helper-use-windows-path "$TOP_RELATIVE")"
# project relative path in IAR-accepted format
IAR_PRJ_RELATIVE="\$PROJ_DIR\$\\\\$(helper-use-windows-path "$PRJ_RELATIVE")"
# LINK relative path in IAR-accepted format
IAR_LINK_RELATIVE="\$PROJ_DIR\$\\\\$(helper-use-windows-path "$LINK_RELATIVE")"

############################################################################
# SCRIPT
############################################################################

generate-ewp $CORE_NAME
generate-eww $CORE_NAME
generate-ewd $CORE_NAME

rm -f "$IAR_PRJ_DIR/.env_gen.sh"

exit 0
