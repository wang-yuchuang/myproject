#!/bin/bash
# calc_xip_membase.sh - 从 ospi2.bpt 自动计算 CONFIG_MEM_BASE 并注入 defconfig
#
# 用法: ./scripts/calc_xip_membase.sh <board> <project> [--emit|--apply] [-s A|B]
#   --emit : 仅输出计算 MEMBASE 到 stdout (供变体生成器; 不改 defconfig, 不要求工程 defconfig 存在)
#   --apply: 更新工程 defconfig 的 CONFIG_MEM_BASE (默认行为, 兼容现状)
#   -s B   : 显式 slot B (等价旧 project=xip_b; 归一化后变体名不再用 xip_b 时使用)
# 示例: ./scripts/calc_xip_membase.sh emu01_a00 xip              # apply (现状行为)
#       ./scripts/calc_xip_membase.sh bmu05_a00 xip --emit       # stdout: 0x081eb000
#       ./scripts/calc_xip_membase.sh bmu05_a00 xip -s B --emit  # stdout: 0x082db000
#
# 原理:
#   MEMBASE = OSPI2_BASE(0x08000000) + ssystem_offset + SFS_HEADER_SIZE(0x2000) + SLOT_OFFSET
#   bpttool 返回的偏移不含 SFS 头部 (0x2000)，运行时 ptdev_get_offset() 含 SFS 头部
#   project=xip_b (slot B) 时 SLOT_OFFSET = ssystem 分区大小 (方案A 按槽链接, design.md D9)

set -e

BOARD=${1:-emu01_a00}
PROJECT=${2:-xip}
# --- 可选参数解析 (兼容原双位置参数调用) ---
EMIT=0
SLOT_ARG=""
shift 2 2>/dev/null || true
while [ $# -gt 0 ]; do
    case "$1" in
        --emit)  EMIT=1 ;;
        --apply) EMIT=0 ;;
        -s)      shift; SLOT_ARG="${1:-}" ;;
        *)       echo "WARNING: unknown arg: $1" >&2 ;;
    esac
    shift
done
OSPI2_BASE=0x08000000

# [ssdk-standalone-boot-mode] 自包含路径解析 (env → ssdk-local → superproject):
#   SSDK_DIR   = .../source/ssdk (本脚本所在子模块根)
#   SD_TOPDIR  = .../d9_sdk 主仓 (仅在 superproject 在场时存在)
SSDK_DIR=$(cd "$(dirname "$0")/.." && pwd)
SD_TOPDIR=$(cd "${SSDK_DIR}/../.." && pwd 2>/dev/null || true)

# BPTTOOL: SSDK_BPTTOOL env → ssdk-local → superproject (meta canonical)
if [ -n "${SSDK_BPTTOOL:-}" ]; then
    BPTTOOL="${SSDK_BPTTOOL}"
elif [ -x "${SSDK_DIR}/tools/bpttool" ]; then
    BPTTOOL="${SSDK_DIR}/tools/bpttool"
else
    BPTTOOL="${SD_TOPDIR}/meta-semidrive/scripts/bpttool"
fi

# BPT_FILE: SSDK_BPT_DIR env (最高) → cfg_industry (superproject) → ssdk-local copy → fail-fast
_ssdk_bpt="${SSDK_DIR}/boards/${BOARD}/secure/pack_cfg/ospi2.bpt"
_cfg_bpt="${SD_TOPDIR}/source/cfg_industry/d9360/projects/${BOARD}/pack_cfg/ospi2.bpt"
if [ -n "${SSDK_BPT_DIR:-}" ]; then
    BPT_FILE="${SSDK_BPT_DIR}/ospi2.bpt"
elif [ -f "${_cfg_bpt}" ]; then
    BPT_FILE="${_cfg_bpt}"
elif [ -f "${_ssdk_bpt}" ]; then
    BPT_FILE="${_ssdk_bpt}"
else
    BPT_FILE="${_cfg_bpt}"
fi

DEFCONFIG="${SSDK_DIR}/boards/${BOARD}/secure/projects/${PROJECT}/defconfig"

if [ ! -f "${BPT_FILE}" ]; then
    echo "ERROR: ospi2.bpt not found: ${BPT_FILE} (set SSDK_BPT_DIR to <pack_cfg dir> or check superproject cfg_industry)" >&2
    exit 1
fi

# defconfig 仅 --apply 模式需要 (--emit 只读 BPT, 供归一化后无物理 xip 目录的变体名使用)
if [ "${EMIT}" != "1" ] && [ ! -f "${DEFCONFIG}" ]; then
    echo "ERROR: defconfig not found: ${DEFCONFIG}"
    exit 1
fi

# 用 bpttool make_table 生成含计算偏移的 JSON，再查询 ssystem 分区偏移
# （query_partition 无法直接处理原始 bpt，因为原始 bpt 只有 size 字符串，无 offset 字段）
TMP_BPT=$(mktemp /tmp/ospi2_expanded_XXXXXX.bpt)
trap 'rm -f "${TMP_BPT}"' EXIT

python "${BPTTOOL}" make_table \
    --input "${BPT_FILE}" \
    --output_json "${TMP_BPT}" 2>/dev/null

SSYSTEM_OFFSET=$(python "${BPTTOOL}" query_partition \
    --input "${TMP_BPT}" \
    --label ssystem \
    --ab_collapse \
    --type offset 2>/dev/null)

SSYSTEM_SIZE=$(python "${BPTTOOL}" query_partition \
    --input "${TMP_BPT}" \
    --label ssystem \
    --ab_collapse \
    --type size 2>/dev/null)

if [ -z "${SSYSTEM_SIZE}" ] || [ "${SSYSTEM_SIZE}" = "None" ]; then
    echo "ERROR: cannot query ssystem size from bpt"
    exit 1
fi

if [ -z "${SSYSTEM_OFFSET}" ] || [ "${SSYSTEM_OFFSET}" = "None" ]; then
    echo "ERROR: cannot query ssystem offset from bpt"
    exit 1
fi

SFS_HEADER_SIZE=0x2000
# [XIP A/B] slot 偏移: slot B 链接地址 = slot A + ssystem 分区大小 (方案A 按槽链接, design.md D9)
#   project==xip_b (旧物理工程名) 或 -s B 均视为 slot B (归一化后变体统一用 -s 参数)
if [ "${SLOT_ARG}" = "B" ] || [ "${PROJECT}" == "xip_b" ]; then
    SLOT_OFFSET=${SSYSTEM_SIZE}
    SLOT_LABEL="B"
else
    SLOT_OFFSET=0
    SLOT_LABEL="A"
fi
# 计算新的 MEMBASE = OSPI2_BASE + ssystem_offset + SFS_HEADER_SIZE + SLOT_OFFSET
# 原因: bpttool 偏移从 BPT 表起始计算，ptdev_get_offset() 从 NOR Flash 物理起始(含 SFS 头部)计算
MEMBASE=$(printf "0x%08x" $((OSPI2_BASE + SSYSTEM_OFFSET + SFS_HEADER_SIZE + SLOT_OFFSET)))

# --emit 模式: 只输出 MEMBASE 值到 stdout, 供 gen_variant_defconfig.sh 注入变体 defconfig
if [ "${EMIT}" == "1" ]; then
    printf "%s\n" "${MEMBASE}"
    exit 0
fi

echo "=== XIP MEMBASE Calculation ==="
echo "  Board:           ${BOARD}"
echo "  Project:         ${PROJECT} (Slot ${SLOT_LABEL})"
echo "  ssystem offset (bpttool): ${SSYSTEM_OFFSET} ($((SSYSTEM_OFFSET)) bytes, 0x$(printf '%x' ${SSYSTEM_OFFSET}))"
echo "  ssystem size (bpttool):   ${SSYSTEM_SIZE} ($((SSYSTEM_SIZE)) bytes, 0x$(printf '%x' ${SSYSTEM_SIZE}))"
echo "  Slot offset:              ${SLOT_OFFSET} ($((SLOT_OFFSET)) bytes)"
echo "  SFS_HEADER_SIZE:           ${SFS_HEADER_SIZE} ($((SFS_HEADER_SIZE)) bytes)"
echo "  OSPI2_BASE:                ${OSPI2_BASE}"
echo "  MEMBASE:                   ${MEMBASE}"

# 更新 defconfig 中的 CONFIG_MEM_BASE (默认/--apply 模式)
OLD_MEMBASE=$(grep "^CONFIG_MEM_BASE=" "${DEFCONFIG}" | head -1 | cut -d= -f2)

if [ "${OLD_MEMBASE}" = "${MEMBASE}" ]; then
    echo "  Status: already correct (${MEMBASE}), no change needed"
else
    sed -i "s/^CONFIG_MEM_BASE=.*/CONFIG_MEM_BASE=${MEMBASE}/" "${DEFCONFIG}"
    echo "  Status: updated ${OLD_MEMBASE:-<unset>} -> ${MEMBASE}"
fi

echo "=== Done ==="
