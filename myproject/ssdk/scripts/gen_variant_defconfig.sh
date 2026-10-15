#!/bin/bash
# gen_variant_defconfig.sh - 由 ref 模板(终态名; 实施期曾名 default) + variants fragment 生成变体 defconfig
# (ssdk-project-mode-unification D2/D2a/D3)
#
# 用法: ./scripts/gen_variant_defconfig.sh <board> <emmc_only|xip> [-s A|B]
#   emmc_only : 模板 + variants/emmc_only.conf (OCRAM 链接)
#   xip       : 模板 + variants/xip.conf + MEM_BASE(calc --emit 由 ospi2.bpt 派生)
#   -s A|B    : xip 槽位 (默认 A); emmc_only 忽略
# 输出: 合成后的完整变体 defconfig 到 stdout (构建脚本重定向到构建期目录)
#
# 原理: Kconfiglib defconfig.py 单文件 load、重复 key 后者胜 -> 模板 + fragment 顺序 cat,
#       同名 bool 以 fragment 的 'is not set'/新值覆盖; int 型差异仅由 fragment 追加定义。
set -e

BOARD=${1:?usage: gen_variant_defconfig.sh <board> <emmc_only|xip> [-s A|B]}
MODE=${2:?mode: emmc_only | xip}
case "${MODE}" in
    emmc_only|xip) ;;
    *) echo "ERROR: mode must be emmc_only | xip (got: ${MODE})" >&2; exit 2 ;;
esac

SLOT_ARG=""
shift 2 2>/dev/null || true
while [ $# -gt 0 ]; do
    case "$1" in
        -s) shift; SLOT_ARG="${1:-A}" ;;
        *)  echo "ERROR: unknown arg: $1" >&2; exit 2 ;;
    esac
    shift
done
[ -z "${SLOT_ARG}" ] && SLOT_ARG="A"

SSDK_DIR=$(cd "$(dirname "$0")/.." && pwd)          # .../source/ssdk
SDK_TOP=$(cd "${SSDK_DIR}/../.." && pwd)            # d9_sdk 顶层
CORE="${SSDK_CORE:-secure}"
TPL_DEF="${SSDK_DIR}/boards/${BOARD}/${CORE}/projects/ref/defconfig"
FRAG="${SSDK_DIR}/boards/${BOARD}/${CORE}/projects/ref/variants/${MODE}.conf"
# [ssdk-standalone-boot-mode] calc 优先 ssdk-local, 回退 superproject (独立构建无 SDK_TOP)
if [ -x "${SSDK_DIR}/scripts/calc_xip_membase.sh" ]; then
    CALC="${SSDK_DIR}/scripts/calc_xip_membase.sh"
else
    CALC="${SDK_TOP}/scripts/calc_xip_membase.sh"
fi

if [ ! -f "${TPL_DEF}" ]; then
    echo "ERROR: template defconfig not found: ${TPL_DEF}" >&2
    exit 1
fi
if [ ! -f "${FRAG}" ]; then
    echo "ERROR: variant fragment not found: ${FRAG}" >&2
    exit 1
fi

# 模板 + fragment (同名 key 由 fragment 覆盖)
cat "${TPL_DEF}"
echo ""
echo "# ==== variant: ${MODE} (fragment) ===="
cat "${FRAG}"

# xip: MEM_BASE 由 NOR 分区表派生注入 (D2a)。calc --emit 不需物理 xip 工程目录存在。
if [ "${MODE}" = "xip" ]; then
    if [ ! -x "${CALC}" ]; then
        echo "ERROR: calc_xip_membase.sh not found/executable: ${CALC}" >&2
        exit 1
    fi
    MEMBASE=$(bash "${CALC}" "${BOARD}" xip -s "${SLOT_ARG}" --emit)
    echo ""
    echo "# ==== MEM_BASE derived from ospi2.bpt (calc --emit, slot ${SLOT_ARG}) ===="
    echo "CONFIG_MEM_BASE=${MEMBASE}"
fi
