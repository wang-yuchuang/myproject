#!/bin/bash

export BOARD=
export PROJECT=
export CORE=
export TOOLCHAIN=
export PYTHONPATH=
export PYTHON=
export OSTYPE

ROOTDIR=$(cd "$( dirname "$0"  )" && pwd)
cd "${ROOTDIR}" || { echo "Error: cannot cd to ROOTDIR: ${ROOTDIR}"; exit 1; }
PREBUILTSDIR=${ROOTDIR}/prebuilts
TOOLSDIR=${ROOTDIR}/tools
PACTOOLDIR=${TOOLSDIR}/sdtools
SSDKTOOLDIR=${TOOLSDIR}/SSDKDS
KCONFIG_LIB=${SSDKTOOLDIR}/scripts/Kconfiglib
AUTOPART_LIB=${SSDKTOOLDIR}/scripts/part
BOARDDIR=
COREDIR=
PRJDIR=
CHIPID=
TOOLCHAIN=gcc
CLEAR=false
MAKE_CONFIG=false
PYTHONPATH=$PYTHONPATH:${ROOTDIR}/tools/sdtools/pactool:${ROOTDIR}/tools/SSDKDS/scripts/Kconfiglib:.
create_package_only=false
CORELISTS=
BOOT_MODE=

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PYTHON=python3
elif [[ "$OSTYPE" == "msys" ]]; then
    PYTHON=${PREBUILTSDIR}/windows/python-3.7.0/python.exe
else
    echo "Unknown OSTYPE $OSTYPE"
    exit 1
fi

# [ssdk-standalone-boot-mode] D9 standalone containerization:
# when invoked directly with -m/--boot-mode on a non-container host, re-exec inside the
# SDK build container via scripts/d9run.sh (--network host + owner-aligned).
# Re-entry safe (d9run.sh sets D9_IN_CONTAINER=1); bypass with SSDK_NO_DOCKER=1.
_ssdk_has_m=0
for _ssdk_arg in "$@"; do
    case "${_ssdk_arg}" in
        -m|--boot-mode|--boot-mode=*) _ssdk_has_m=1 ;;
    esac
done
if [ "${_ssdk_has_m}" = "1" ] \
    && [ "${SSDK_NO_DOCKER:-0}" != "1" ] \
    && [ "${SSDK_DELEGATED_BUILD:-0}" != "1" ] \
    && [ ! -e /.dockerenv ] \
    && [ "${D9_IN_CONTAINER:-0}" != "1" ] \
    && command -v docker >/dev/null 2>&1 \
    && [ -x "${SSDK_D9RUN:-${ROOTDIR}/../../scripts/d9run.sh}" ]; then
    exec "${SSDK_D9RUN:-${ROOTDIR}/../../scripts/d9run.sh}" "${ROOTDIR}/build.sh" "$@"
fi

BOARDS=(\
        "e3_evb" \
        "e3_gateway" \
        "e3_motor" \
        "e3_disp" \
        "e3_144_evb" \
        "e3_144_ref" \
        "e3_176_evb" \
        "e3_176_ref" \
        "d3_disp" \
        "d3_gateway" \
        "x9h_ref" \
        "d9360_ref" \
        "d9342_ref" \
        "d9_ref" \
        "d9310_ref" \
        "d9350_ref" \
        "x9hp_ms" \
        "x9hp_ms_a12_vemmc" \
        "x9hp_ms_x9sp_ea" \
        "x9hp_ms_v9p_ea" \
        "x9sp_ms" \
        "x9cc_ms_a" \
        "x9cc_ms_b" \
        "x9sp_ms_demo" \
        "x9sp_evb" \
        "v9p_ms" \
        "g9x_ref" \
        "taishan_z1" \
        "taishan_haps" \
        "taishan_tianjie" \
        "taishan_l_z1" \
        "taishan_ll_z1" \
        "x9h_ms" \
        "x9e_ref" \
        "x9_ref_serdes" \
        "x9m_ref_serdes" \
        "x9plus_ref_serdes" \
        "x9u-ref_a" \
        "x9u-ref_b" \
        "bmu03_a00" \
        "bmu02_a00" \
        "bmu05_a00" \
        "bmu05_t00" \
        "emu01_a00" \
        )

PROJECTS=(\
        "
        e3_evb/ref
        e3_evb/validation
        e3_evb/dloader
        e3_evb/flashloader
        " \
        "
        e3_gateway/ref
        e3_gateway/xip
        e3_gateway/dloader
        e3_gateway/bootloader
        e3_gateway/flashloader
        e3_gateway/baremetal
        " \
        "
        e3_motor/baremetal
        e3_motor/dloader
        e3_motor/bootloader
        e3_motor/flashloader
        " \
        "
        e3_disp/ref
        e3_disp/cluster
        e3_disp/cluster_88
        e3_disp/cluster_1920x720
        e3_disp/dloader
        e3_disp/flashloader
        " \
        "
        e3_144_evb/ref
        e3_144_evb/dloader
        " \
        "
        e3_144_ref/ref
        e3_144_ref/xip
        e3_144_ref/dloader
        e3_144_ref/bootloader
        e3_144_ref/flashloader
        e3_144_ref/baremetal
        " \
        "
        e3_176_evb/ref
        e3_176_evb/dloader
        " \
        "
        e3_176_ref/ref
        e3_176_ref/xip
        e3_176_ref/dloader
        e3_176_ref/bootloader
        e3_176_ref/flashloader
        e3_176_ref/baremetal
        " \
        "
        d3_disp/ref
        d3_disp/cluster
        d3_disp/cluster_88
        d3_disp/dloader
        " \
        "
        d3_gateway/ref
        d3_gateway/xip
        d3_gateway/dloader
        d3_gateway/bootloader
        d3_gateway/flashloader
        d3_gateway/baremetal
        " \
        "
        x9h_ref/ref
        " \
        "
        d9360_ref/ref
        d9360_ref/lowerpoweros
        " \
        "
        d9342_ref/ref
        " \
        "
        d9_ref/ref
        d9_ref/h001
        d9_ref/h002
        d9_ref/lowerpoweros
        " \
        "
        d9310_ref/ref
        d9310_ref/g001
        " \
        "
        d9350_ref/ref
        " \
        "
        x9hp_ms/ms
        x9hp_ms/ms-mmc
        " \
        "
        x9hp_ms_a12_vemmc/ms
        x9hp_ms_a12_vemmc/ms-mmc
        x9hp_ms_a12_vemmc/ms-parallel
        x9hp_ms_a12_vemmc/ms-vmmc
        x9hp_ms_a12_vemmc/lowerpoweros
        " \
        "
        x9hp_ms_x9sp_ea/ms
        x9hp_ms_x9sp_ea/ms-mmc
        x9hp_ms_x9sp_ea/ms-parallel
        x9hp_ms_x9sp_ea/ms-vmmc
        x9hp_ms_x9sp_ea/lowerpoweros
        " \
        "
        x9hp_ms_v9p_ea/ms
        x9hp_ms_v9p_ea/ms-mmc
        x9hp_ms_v9p_ea/ms-vmmc
        " \
        "
        x9sp_ms/ms
        x9sp_ms/ms-mmc
        x9sp_ms/ms-parallel
        x9sp_ms/ms-vmmc
        x9sp_ms/ms-ahub
        x9sp_ms/ms-saf-mcal
        " \
        "
        x9cc_ms_a/ms
        x9cc_ms_a/ms-mmc
        x9cc_ms_a/ms-parallel
        x9cc_ms_a/ms-vmmc
        x9cc_ms_a/ms-ahub
        x9cc_ms_a/lowerpoweros
        " \
        "
        x9cc_ms_b/ms
        x9cc_ms_b/ms-mmc
        x9cc_ms_b/ms-parallel
        x9cc_ms_b/ms-vmmc
        x9cc_ms_b/lowerpoweros
        " \
        "
        x9sp_ms_demo/ms
        x9sp_ms_demo/ms-mmc
        x9sp_ms_demo/ms-parallel
        x9sp_ms_demo/ms-vmmc
        " \
        "
        x9sp_evb/evb
        x9sp_evb/evb-mmc
        x9sp_evb/evb-parallel
        x9sp_evb/evb-vmmc
        " \
        "
        v9p_ms/ms
        v9p_ms/ms-mmc
        v9p_ms/ms-vmmc
        v9p_ms/ms-parallel
        " \
        "
        g9x_ref/ref
        g9x_ref/flashloader
        " \
        "
        taishan_z1/validation
        " \
        "
        taishan_haps/validation
        " \
        "
        taishan_tianjie/validation
        " \
        "
        taishan_l_z1/validation
        " \
        "
        taishan_ll_z1/validation
        " \
        "
        x9h_ms/ms
        " \
        "
        x9e_ref/ref
        " \
        "
        x9_ref_serdes/ref
        " \
        "
        x9m_ref_serdes/ref
        " \
        "
        x9plus_ref_serdes/ref
        " \
        "
        x9u-ref_a/ref
        x9u-ref_a/ref-mmc
        x9u-ref_a/xip
        " \
        "
        x9u-ref_b/ref
        x9u-ref_b/ref-mmc
        x9u-ref_b/xip
        " \
        "
        bmu03_a00/ref
        " \
        "
        bmu02_a00/ref
        " \
        "
        bmu05_a00/ref
        bmu05_a00/xip
        bmu05_a00/xip_b
        " \
        "
        bmu05_t00/ref
        bmu05_t00/xip
        bmu05_t00/xip_b
        " \
"
emu01_a00/ref
emu01_a00/xip
emu01_a00/xip_b
" \
        )

function get_project_list()
{
    for (( i = 0; i < ${#BOARDS[*]}; i++)); do
        if [ $1 == ${BOARDS[$i]} ]; then
            echo ${PROJECTS[$i]}
            break
        fi
    done
}

function usage()
{
    local i=1 j=1

    echo -e "\nUsage: build.sh
    Optional parameters: [-b board] [-p project] [-c core] [-m boot-mode]"
echo "
    * [-b board]:            board name
    * [-p project]:          project name
    * [-m boot-mode]:        boot mode: emmc | xip (equals-form: --boot-mode=<emmc|xip>)
    *                        emmc: OCRAM single image + legacy bundle
    *                        xip:  NOR A/B dual-slot; one invocation builds both slots
    *                              and produces a per-slot bundle
    *                        -m is the boot mode (semantic), -p is the project template
    *                        directory (structural); they are distinct axes. -m with an
    *                        inconsistent -p is an error. Only valid with -c secure and
    *                        boards that have ref/variants/.
    * [-c core]:             core name[optional, will build all core when not specified]
    * [-iar]:                generate iar project
    * [-clear]:              clear out objects before make
    * [-pac]:                only create images package
    * [-h]:                  This help message
    * [-a args]:             project config arguements
"
    echo -e "Boards & Projects:"
    for board in ${BOARDS[*]}; do
        echo "      ($i) ${board}" && let i++
        project_list=`get_project_list ${board}`
        j=1
        for project in ${project_list[*]}; do
            echo "            ($j) ${project#*/}" && let j++
        done
    done
    echo -e "\n----------------------------------------------------"
}

# [ssdk-standalone-boot-mode] 逐字等价四态版本生成器 (superproject meta-semidrive/scripts/auto_version.sh 的内联回退)。
# 仅当 superproject 脚本不可达 (子模块独立构建) 时使用; 输出必须与 auto_version.sh plain 格式逐字节一致。
# 返回码: 0 成功; 2 prefix/length 断言失败。
function ssdk_inline_auto_version()
{
    local dir="$1" prefix="$2"
    local full_prefix="" body="" sha8="" tag="" describe="" is_git=false dirty=false

    # prefix 断言: {CHIP}-{BOARD} (两个非空 token, 内部无 '-')
    if [ -n "$prefix" ] && [[ ! "$prefix" =~ ^[^-]+-[^-]+$ ]]; then
        echo "ssdk_inline_auto_version: invalid prefix '$prefix' (must be {CHIP}-{BOARD}, tokens MUST NOT contain '-')" >&2
        return 2
    fi

    if [ -n "$prefix" ]; then
        full_prefix="${prefix}-"
    fi

    if git -C "$dir" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        is_git=true
    fi

    if [ "$is_git" = true ]; then
        sha8="$(git -C "$dir" rev-parse --short=8 HEAD 2>/dev/null || true)"
        tag="$(git -C "$dir" describe --exact-match --tags HEAD 2>/dev/null || true)"
        describe="$(git -C "$dir" describe --tags --always HEAD 2>/dev/null || true)"
        if git -C "$dir" diff-index --quiet HEAD -- 2>/dev/null; then
            dirty=false
        else
            dirty=true
        fi
    fi

    # BODY 四态
    if [ "$is_git" != true ]; then
        body="dev-unknown"
    elif [ -n "$tag" ]; then
        if [ "$dirty" = true ]; then
            body="${tag}-${sha8}-dirty"
        else
            body="${tag}"
        fi
    else
        if git -C "$dir" describe --tags HEAD >/dev/null 2>&1; then
            if [ "$dirty" = true ]; then
                body="dev-${describe}-dirty"
            else
                body="dev-${describe}"
            fi
        else
            if [ "$dirty" = true ]; then
                body="dev-${sha8}-dirty"
            else
                body="dev-${sha8}"
            fi
        fi
    fi

    local version="${full_prefix}${body}"

    if [ "${#version}" -gt 63 ]; then
        echo "ssdk_inline_auto_version: version string length ${#version} exceeds 63: '${version}'" >&2
        return 2
    fi

    printf '%s\n' "$version"
    return 0
}


# Function to generate version header for SSDK
function generate_ssdk_version()
{
    local BOARD=$1
    local version_file="${ROOTDIR}/include/ssdk_version.h"

    # unify-version-management: 权威版本由 superproject meta-semidrive/scripts/auto_version.sh 生成 (Decision 1/2)。
    # BSP_PREFIX 由 project_build_link.sh 导出; 独立构建时回退 D9360-${BOARD}。
    # superproject 脚本不可达 (子模块独立构建) 时回退逐字等价内联生成器 (ssdk_inline_auto_version)。
    local prefix="${BSP_PREFIX:-D9360-${BOARD}}"
    local auto_version_script="${ROOTDIR}/../../meta-semidrive/scripts/auto_version.sh"

    local version
    if [ -x "${auto_version_script}" ]; then
        version="$("${auto_version_script}" --dir "${ROOTDIR}" --prefix "${prefix}" 2>/dev/null)" || {
            echo "generate_ssdk_version: auto_version.sh failed for ${ROOTDIR}" >&2
            exit 1
        }
    else
        echo "generate_ssdk_version: ${auto_version_script} not found/executable; using byte-equivalent inline fallback" >&2
        version="$(ssdk_inline_auto_version "${ROOTDIR}" "${prefix}")" || {
            echo "generate_ssdk_version: inline version generator failed for ${ROOTDIR}" >&2
            exit 1
        }
    fi

    local commit=$(git -C "${ROOTDIR}" rev-parse --short=8 HEAD 2>/dev/null || echo "unknown")
    local build_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

    # Create include directory if needed
    mkdir -p "${ROOTDIR}/include"

    cat > ${version_file} << EOF
#ifndef __SSDK_VERSION_H__
#define __SSDK_VERSION_H__

#define SSDK_VERSION "${version}"
#define SSDK_COMMIT "${commit}"
#define SSDK_BUILD_TIME "${build_time}"

#endif
EOF
    echo "Generated SSDK version: ${version}"
}

function is_taishan_platform()
{
    local BOARD=$1
    local result=0

    if [[ "${BOARD}" == e3* ]] || [[ "${BOARD}" == d3* ]]; then
        result=1
    fi

    echo $result
    return $?
}

function get_taishan_core_lists()
{
    local BOARD=$1
    local PROJECT=$2
    local CORE='sf'

    PRJDIR=${ROOTDIR}/boards/${BOARD}/sf/projects/${PROJECT}
    CORELISTS=`$PYTHON ${AUTOPART_LIB}/part_get_cores.py --kconfig_path=${ROOTDIR} \
                --defconfig_path=${PRJDIR} --out_path=${PRJDIR}`
}

function get_core_list()
{
    local BOARD=$1
    local PROJECT=$2

    if [ $(is_taishan_platform ${BOARD}) == 1 ]; then
        get_taishan_core_lists ${BOARD} ${PROJECT}
    fi
}

function is_core_present()
{
    local BOARD=$1
    local PROJECT=$2
    local CORE=$3
    local result=1

    if [ $(is_taishan_platform ${BOARD}) == 1 ]; then
        if [[ ${CORELISTS} != *$CORE* ]]; then
            result=0
        fi
    fi

    echo $result
    return $?
}

function project_build()
{
    local BOARD=$1
    local PROJECT=$2
    local CORE=$3
    local ARGS="$4"
    
    # === 新增：版本注入 ===
    generate_ssdk_version ${BOARD}
    # ========================
    # === 归一化变体工程 (ssdk-project-mode-unification D2/D4) ===
    # 激活: CORE 的 projects/ref 含 variants/ (模板标志, 终态目录名 ref) 且 -p ∈ ref/xip/xip_b;
    #   生成变体 defconfig 到 out/variant_defconfig/ (gitignore 构建区, 零 dirty);
    #   defconfig.py 读生成文件而非工程 defconfig; calc --apply 跳过 (生成器已 calc --emit 注入 MEM_BASE);
    # 未归一板 (无 variants/) 走原路径。
    VARIANT_DEFCONFIG=""
    V_TEMPLATE_DIR=""
    if [ -f "${ROOTDIR}/boards/${BOARD}/${CORE}/projects/ref/variants/emmc_only.conf" ]; then
        case "${PROJECT}" in
            ref)   V_MODE="emmc_only"; V_SLOT="" ;;
            xip)   V_MODE="xip";       V_SLOT="A" ;;
            xip_b) V_MODE="xip";       V_SLOT="B" ;;
            *)     V_MODE="" ;;
        esac
        if [ -n "${V_MODE}" ]; then
            V_DIR="${ROOTDIR}/out/variant_defconfig/${BOARD}/${PROJECT}"
            mkdir -p "${V_DIR}"
            if SSDK_CORE="${CORE}" bash "${ROOTDIR}/scripts/gen_variant_defconfig.sh" "${BOARD}" "${V_MODE}" \
                    ${V_SLOT:+-s ${V_SLOT}} > "${V_DIR}/defconfig"; then
                VARIANT_DEFCONFIG="${V_DIR}/defconfig"
                V_TEMPLATE_DIR="${ROOTDIR}/boards/${BOARD}/${CORE}/projects/ref"
                echo "[variant] ${PROJECT} -> ref+${V_MODE}.conf${V_SLOT:+(-s ${V_SLOT})}: ${VARIANT_DEFCONFIG}"
            elif [ "${SSDK_ALLOW_STALE_MEMBASE:-0}" = "1" ]; then
                # D2b 调试逃生门 (审计修正): 丢弃残缺/空生成文件, 回退下方 calc --apply 以 stale MEM_BASE 继续
                #   (xip/xip_b: 模板 defconfig 无 CONFIG_MEM_BASE 行, calc sed 不新增 -> make 落 Kconfig 默认
                #    0x3B000000 DDR——链接地址错误, 仅限调试)。emmc_only/ref 无 calc 回退, 直接失败。
                echo "WARN[${PROJECT}]: variant defconfig generation failed; SSDK_ALLOW_STALE_MEMBASE=1 -> fallback calc --apply (stale MEM_BASE, DANGEROUS)" >&2
                rm -f "${V_DIR}/defconfig"
                if [ "${PROJECT}" != "xip" ] && [ "${PROJECT}" != "xip_b" ]; then
                    echo "ERROR: ${PROJECT} has no safe fallback under hatch; abort (${BOARD}/${PROJECT})" >&2
                    exit 1
                fi
            else
                echo "ERROR: variant defconfig generation failed (${BOARD}/${PROJECT})" >&2
                exit 1
            fi
    fi
    fi

    # === XIP: 自动计算 CONFIG_MEM_BASE (未归一板保留 --apply; 归一化由变体生成器处理) ===
    if [ -z "${VARIANT_DEFCONFIG}" ] && { [ "${PROJECT}" == "xip" ] || [ "${PROJECT}" == "xip_b" ]; }; then
        # [ssdk-standalone-boot-mode] calc 优先 ssdk-local, 回退 superproject (独立构建无 SDK_TOP)
        if [ -x "${ROOTDIR}/scripts/calc_xip_membase.sh" ]; then
            CALC_SCRIPT="${ROOTDIR}/scripts/calc_xip_membase.sh"
        else
            SDK_TOP_CALC=$(cd "${ROOTDIR}/../.." && pwd)
            CALC_SCRIPT="${SDK_TOP_CALC}/scripts/calc_xip_membase.sh"
        fi
        if [ -x "${CALC_SCRIPT}" ]; then
            echo "[XIP] Calculating MEMBASE from ospi2.bpt..."
            bash "${CALC_SCRIPT}" "${BOARD}" "${PROJECT}" || echo "[XIP] WARNING: calc_xip_membase failed"
        fi
    fi
    
    COREDIR=${ROOTDIR}/boards/${BOARD}/${CORE}
    # 归一化激活时 PRJDIR 统一为 ref 模板实体 (终态: 无 xip/xip_b 物理目录/symlink, D6)
    PRJDIR=${V_TEMPLATE_DIR:-${COREDIR}/projects/${PROJECT}}
    # 传递模板目录给 make (engine.mk PRJDIR 覆盖), 使 main.c/project.mk/.config 从 ref 实体读;
    # BUILDDIR 仍用 PROJECT 别名 (build/xip/... 产物目录独立)
    if [ -n "${V_TEMPLATE_DIR}" ]; then
        export PROJECT_SRC_DIR="${V_TEMPLATE_DIR}"
    else
        unset PROJECT_SRC_DIR
    fi

    KCONFIG_TOP=${ROOTDIR}/Kconfig
    KCONFIG_DEFCONFIG=${VARIANT_DEFCONFIG:-${PRJDIR}/defconfig}
    KCONFIG_HEADER=${PRJDIR}/config.h
    KCONFIG_MK=${PRJDIR}/.config
    KCONFIG_CONFIG=${KCONFIG_MK} ${PYTHON} ${KCONFIG_LIB}/defconfig.py --kconfig ${KCONFIG_TOP} ${KCONFIG_DEFCONFIG}
    KCONFIG_CONFIG=${KCONFIG_DEFCONFIG} ${PYTHON} ${KCONFIG_LIB}/genconfig.py --header-path ${KCONFIG_HEADER} ${KCONFIG_TOP}

    if [ $(is_core_present ${BOARD} ${PROJECT} ${CORE}) == 1 ]; then
        echo -e "\nBuild [Board]: ${BOARD} [Core]: ${CORE} [Project]: ${PROJECT}\n"

        if [ ${PROJECT} == flashloader ]; then
            TOOLCHAIN=iar
        fi

        if [ ${CLEAR} == true ]; then
            make clean
        fi

        if [ ${MAKE_CONFIG} == true ]; then
            make make_configs
        elif [ ${TOOLCHAIN} == iar ]; then
            if [ ${PROJECT} == dloader ]; then
                local orig_toolchain=${TOOLCHAIN}
                TOOLCHAIN=gcc
                make ${ARGS} -j8
                TOOLCHAIN=${orig_toolchain}
            else
                make iar
            fi
        elif [ ${TOOLCHAIN} == rapidstudio ]; then
            if [ ${PROJECT} == dloader ]; then
                local orig_toolchain=${TOOLCHAIN}
                TOOLCHAIN=gcc
                make ${ARGS} -j8
                TOOLCHAIN=${orig_toolchain}
            else
                make rapidstudio
            fi
        else
            make ${ARGS} -j8
        fi
        exit_code=$?

        if [ ${exit_code} != 0 ]; then
            echo -e "\nBuild Fail: [Board]: ${BOARD} [Core]: ${CORE} [Project]: ${PROJECT}\n"
            exit ${exit_code}
        fi
    fi
}

function project_pac()
{
    if [ $(is_taishan_platform ${BOARD}) == 1 ]; then
        project_build ${BOARD} dloader sf
        echo -e "\nPac [Board]: ${BOARD} [Project]: ${PROJECT}\n"
        ${PYTHON} ${PWD}/tools/genpac.py -b ${BOARD} -p ${PROJECT} -v ${TOOLCHAIN}
        exit_code=$?
        if [ ${exit_code} != 0 ]; then
            echo -e "\Pac Fail: [Board]: ${BOARD} [Project]: ${PROJECT}\n"
            exit ${exit_code}
        fi
    fi
}

function generate_multiprj_eww()
{
    local BOARD=$1
    local PROJECT=$2
    local path=${ROOTDIR}/boards/${BOARD}/build/${PROJECT}/iar
    local tpl="$path/${PROJECT}.eww"

    if [ -f $tpl ]; then
        rm -f $tpl
    fi

    (
        echo -e "<?xml version="1.0" encoding="iso-8859-1"?>"
        echo -e "<workspace>"
        for core_name in `ls ${path}`; do
            if [ -f ${path}/${core_name}/${core_name}.ewp ] && [ $(is_core_present ${BOARD} ${PROJECT} ${core_name}) == 1 ]; then
                echo -e "  <project>"
                echo -e "    <path>\$WS_DIR\$\\\\${core_name}\\\\${core_name}.ewp</path>"
                echo -e "  </project>"
            fi
        done
        echo -e "</workspace>"
    ) > "$tpl"
}

function generate_ide_prj()
{
    local path=$1
    local tpl="$path/.project"

    if [ -f $tpl ]; then
        rm -f $tpl
    fi

    (
        echo -e "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        echo -e "<projectDescription>"
        echo -e "	<name>${BOARD}_${PROJECT}</name>"
        echo -e "	<comment></comment>"
        echo -e "	<projects>"
        echo -e "	</projects>"
        echo -e "	<buildSpec>"
        echo -e "	</buildSpec>"
        echo -e "	<natures>"
        echo -e "	</natures>"
        echo -e "</projectDescription>"
    ) > "$tpl"
}

if [ -z "$1" ];then
usage
exit 0
fi

while [ ! -z "$1" ]; do
    case $1 in
        -b )
            shift
            for board in ${BOARDS[*]}; do
                if [ $1 == ${board} ]; then
                    projects=`get_project_list $1`
                    BOARD=$1
                fi
            done
            if [ "${BOARD}" == "" ]; then
                echo "Error: Unable to find the board from $1"
                exit 0
            fi
            ;;
        -p )
            shift
            for project in ${projects[*]}; do
                if [ $1 == ${project#*/} ]; then
                    PROJECT=$1
                fi
            done
            if [ "${PROJECT}" == "" ]; then
                echo "Error: Unable to find the project from $1"
                exit 1
            fi
            ;;
        -c )
            shift
            CORE=$1
            ;;
        -m|--boot-mode )
            shift
            BOOT_MODE=$1
            case "${BOOT_MODE}" in
                emmc|xip) ;;
                *) echo "Error: -m/--boot-mode must be one of {emmc,xip} (got: ${BOOT_MODE})"; exit 1 ;;
            esac
            ;;
        --boot-mode=* )
            BOOT_MODE="${1#--boot-mode=}"
            case "${BOOT_MODE}" in
                emmc|xip) ;;
                *) echo "Error: -m/--boot-mode must be one of {emmc,xip} (got: ${BOOT_MODE})"; exit 1 ;;
            esac
            ;;
        -a )
            shift
            PROJECT_CONFIG_ARGS="$1"
            ;;
        -clear )
            CLEAR=true
            ;;
        -chipid )
            shift
            CHIPID=$1
            ;;
        -iar )
            TOOLCHAIN=iar
            ;;
        -rapidstudio )
            TOOLCHAIN=rapidstudio
            ;;
        -make_configs )
            MAKE_CONFIG=true
            ;;
        -pac )
            create_package_only=true
            ;;
        -h )
            usage
            exit 0
            ;;
        * )
    esac
    shift
done

# [ssdk-standalone-boot-mode] 前置清单检查 (D5): 逐项输出 pass/fail, 替代散落的隐式失败。
# 非致命警告; 唯 xip 缺 BPT 为硬失败 (return 1)。
function ssdk_preflight()
{
    local board="$1" mode="$2"
    echo "=== SSDK preflight (board=${board}, mode=${mode}) ==="

    # (1) cross toolchain presence (SEC 域 Cortex-R5 → arm-none-eabi; 兼容 arm64 aarch64-elf)
    if [ -x "${PREBUILTSDIR}/linux/gcc/arm/bin/arm-none-eabi-gcc" ]; then
        echo "[preflight] toolchain: PASS (${PREBUILTSDIR}/linux/gcc/arm/bin/arm-none-eabi-gcc)"
    elif [ -x "${PREBUILTSDIR}/linux/gcc/arm64/bin/aarch64-elf-gcc" ]; then
        echo "[preflight] toolchain: PASS (${PREBUILTSDIR}/linux/gcc/arm64/bin/aarch64-elf-gcc)"
    else
        echo "[preflight] toolchain: FAIL (no cross gcc under ${PREBUILTSDIR}/linux/gcc)" >&2
    fi

    # (2) python resolves and is python2 (bpttool 需要 python2; python3 仅告警)
    if command -v python >/dev/null 2>&1; then
        local _py_major
        _py_major="$(python --version 2>&1 | sed -E 's/^Python ([0-9]+).*/\1/')"
        if [ "${_py_major}" = "2" ]; then
            echo "[preflight] python: PASS (python2)"
        else
            echo "[preflight] python: WARN (python resolves to python${_py_major}; bpttool needs python2 — run inside Ubuntu 18.04 container)" >&2
        fi
    else
        echo "[preflight] python: FAIL (no 'python' on PATH; bpttool needs python2)" >&2
    fi

    # (3) BPT reachable (仅 xip 模式需要; 缺失为硬失败)
    if [ "${mode}" = "xip" ]; then
        local bpt_path
        local _ssdk_bpt="${ROOTDIR}/boards/${board}/secure/pack_cfg/ospi2.bpt"
        local _cfg_bpt="${ROOTDIR}/../../source/cfg_industry/d9360/projects/${board}/pack_cfg/ospi2.bpt"
        if [ -n "${SSDK_BPT_DIR:-}" ]; then
            bpt_path="${SSDK_BPT_DIR}/ospi2.bpt"
        elif [ -f "${_cfg_bpt}" ]; then
            bpt_path="${_cfg_bpt}"
        elif [ -f "${_ssdk_bpt}" ]; then
            bpt_path="${_ssdk_bpt}"
        else
            bpt_path="${_cfg_bpt}"
        fi
        if [ -f "${bpt_path}" ]; then
            echo "[preflight] BPT: PASS (${bpt_path})"
        else
            echo "[preflight] BPT: FAIL (${bpt_path}; set SSDK_BPT_DIR to <pack_cfg dir>)" >&2
            return 1
        fi
    fi

    # (4) remote signing server reachable (仅 bundle 阶段; SSDK_SKIP_BUNDLE=1 跳过)
    if [ "${SSDK_SKIP_BUNDLE:-0}" != "1" ] && command -v curl >/dev/null 2>&1; then
        local sign_url="${REMOTE_SIGN_URL:-http://192.168.34.207:8900}"
        if curl -s --max-time 5 -o /dev/null "${sign_url}/api/v1/cert/vbmeta_signing" 2>/dev/null; then
            echo "[preflight] signing server: PASS (${sign_url})"
        else
            echo "[preflight] signing server: WARN (${sign_url} unreachable; bundle stage will fail)" >&2
        fi
    fi

    echo "=== preflight done ==="
    return 0
}

# === ssdk-standalone-boot-mode: -m/--boot-mode resolution ===
# -m is the boot-mode semantic axis; -p is the physical project template axis. They are distinct.
PROJECT_FROM_P=""
case "${PROJECT}" in
    ref)   PROJECT_FROM_P="emmc" ;;
    xip)   PROJECT_FROM_P="xip"  ;;
    xip_b) PROJECT_FROM_P="xip"  ;;
esac
if [ -n "${BOOT_MODE}" ]; then
    ssdk_preflight "${BOARD}" "${BOOT_MODE}" || exit 1
    # conflict: -p alias implies a boot mode different from -m
    if [ -n "${PROJECT_FROM_P}" ] && [ "${PROJECT_FROM_P}" != "${BOOT_MODE}" ]; then
        echo "Error: -m ${BOOT_MODE} conflicts with -p ${PROJECT} (implies ${PROJECT_FROM_P})"
        exit 1
    fi
    # -m only defines secure-core semantics
    if [ -n "${CORE}" ] && [ "${CORE}" != "secure" ]; then
        echo "Error: -m/--boot-mode is only valid for -c secure (got: ${CORE})"
        exit 1
    fi
    CORE="secure"
    # -b is required for -m
    if [ -z "${BOARD}" ]; then
        echo "Error: -m/--boot-mode requires -b <board>"
        exit 1
    fi
    # generic variant detection (no hardcoded board name)
    if [ ! -f "${ROOTDIR}/boards/${BOARD}/${CORE}/projects/ref/variants/emmc_only.conf" ]; then
        echo "Error: board ${BOARD} has no ref/variants/; -m/--boot-mode unsupported here, use -p <project>"
        exit 1
    fi
    # map boot mode to legacy project alias (emmc->ref template, xip->xip slot A; slot B built separately)
    if [ -z "${PROJECT}" ]; then
        case "${BOOT_MODE}" in
            emmc) PROJECT="ref" ;;
            xip)  PROJECT="xip" ;;
        esac
    fi
    # -m requires a variant-capable project alias (ref/xip/xip_b); reject others (e.g. lowerpoweros)
    case "${PROJECT}" in
        ref|xip|xip_b) ;;
        *) echo "Error: -m/--boot-mode requires project in {ref,xip,xip_b} (got: ${PROJECT})"; exit 1 ;;
    esac
fi

BOARDDIR=${ROOTDIR}/boards/${BOARD}

if [ -d ${BOARDDIR} ]; then
    if [ -n "${CHIPID}" ];then
        source ${AUTOPART_LIB}/autopartgen.sh ${CHIPID} ${PROJECT}
    fi

    # [ssdk-standalone-boot-mode] -m: clear target build dirs to avoid stale .o (parity with top-level do_compile_ssdk)
    if [ "${BOOT_MODE}" = "emmc" ]; then
        rm -rf "${BOARDDIR}/build/ref"
    elif [ "${BOOT_MODE}" = "xip" ]; then
        rm -rf "${BOARDDIR}/build/xip" "${BOARDDIR}/build/xip_b"
    fi

    get_core_list ${BOARD} ${PROJECT}

    if [ "$create_package_only" == "true" ];then
        project_pac
        exit 0
    elif [ "${CORE}" == "" ] && [ ! -e ${BOARDDIR}/projects ]; then
        for core_name in `ls ${BOARDDIR}`; do
            if [ -e ${BOARDDIR}/${core_name}/projects/${PROJECT} ]; then
                CORE=${core_name}
                project_build ${BOARD} ${PROJECT} ${CORE} "${PROJECT_CONFIG_ARGS}"
                if [ "${CORE}" == "sf" ] && [ -e ${BOARDDIR}/${core_name}/projects/bootloader ];then
                    project_build ${BOARD} bootloader ${CORE} "${PROJECT_CONFIG_ARGS}"
                fi
            fi
        done
        if [ ${TOOLCHAIN} == iar ];then
            generate_multiprj_eww ${BOARD} ${PROJECT}
        elif [ ${TOOLCHAIN} == rapidstudio ];then
            generate_ide_prj ${BOARDDIR}/build/${PROJECT}/${TOOLCHAIN}
        else
            project_pac
        fi
    else
        project_build  ${BOARD} ${PROJECT} ${CORE} "${PROJECT_CONFIG_ARGS}"
    fi

    # [ssdk-standalone-boot-mode] -m xip: build BOTH slots A (xip) and B (xip_b)
    if [ "${BOOT_MODE}" = "xip" ]; then
        if [ "${PROJECT}" != "xip" ]; then
            echo "[mode] xip: dual-link slot A (xip)"
            project_build ${BOARD} xip ${CORE} "${PROJECT_CONFIG_ARGS}"
        fi
        if [ "${PROJECT}" != "xip_b" ]; then
            echo "[mode] xip: dual-link slot B (xip_b)"
            project_build ${BOARD} xip_b ${CORE} "${PROJECT_CONFIG_ARGS}"
        fi
    fi

    # [ssdk-standalone-boot-mode] auto-OTA gating:
    #  - -m xip: builds per-slot bundle (stage-2 contract)
    #  - delegated compile (top-level sets SSDK_DELEGATED_BUILD=1): no auto-bundle (top-level bundles once)
    #  - standalone -m: ignores ambient SSDK_NO_AUTO_OTA (env scoping, D7/N1); legacy -p honors it
    _ssdk_auto_ota=0
    if [ "$CORE" = "secure" ] \
        && [ "${SSDK_DELEGATED_BUILD}" != "1" ] \
        && [ -f "${ROOTDIR}/scripts/ota-bundle.sh" ] \
        && [ "$create_package_only" != "true" ]; then
        if [ -n "${BOOT_MODE}" ] || [ "$SSDK_NO_AUTO_OTA" != "1" ]; then
            _ssdk_auto_ota=1
        fi
    fi
    if [ "$_ssdk_auto_ota" = "1" ]; then
        echo -e "\n[SSDK-OTA] Building RAUC OTA bundle..."
        SSDK_BOARD="${BOARD}" \
        SSDK_PROJECT="${PROJECT}" \
        SSDK_BUILD_TYPE="secure" \
        SSDK_BOOT_MODE="${BOOT_MODE}" \
        SSDK_OUTPUT_DIR="${SSDK_OUTPUT_DIR:-${ROOTDIR}/out${BOOT_MODE:+/${BOARD}/${BOOT_MODE}}}" \
        bash "${ROOTDIR}/scripts/ota-bundle.sh"
    fi
else
    echo "Error: Unable to find the board dir"
fi
