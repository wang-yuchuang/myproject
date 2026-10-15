#!/bin/bash
# check_vendored_tools.sh — 防漂移断言: ssdk vendored 工具与 meta-semidrive canonical 副本 byte-identity。
#
# [ssdk-standalone-boot-mode] ssdk 已将 bpttool/avbtool/remote_sign_helper_with_files vendor 至
# source/ssdk/tools/。修改这些工具时必须双侧同步提交, 否则 OTA vbmeta 与主仓 pack vbmeta 行为分叉
# (验签失败)。本脚本在 superproject 在场时逐字节比对; 任一失配 exit 1。
#
# NOTE: 应挂入 d9_check_all.sh 首步 (superproject 在场时自动执行)。
set -u

SSDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SD_TOPDIR="$(cd "${SSDK_DIR}/../.." && pwd 2>/dev/null || true)"

META_DIR="${SD_TOPDIR}/meta-semidrive/scripts"

# <vendored_rel> <meta_rel> <label>
check_one() {
    local vendored="$1" meta="$2" label="$3"
    local v_path="${SSDK_DIR}/${vendored}"
    local m_path="${META_DIR}/${meta}"

    if [ ! -f "$v_path" ]; then
        echo "FAIL: ${label}: vendored copy missing: ${v_path}"
        return 1
    fi
    if [ ! -f "$m_path" ]; then
        echo "FAIL: ${label}: meta counterpart missing: ${m_path}"
        return 1
    fi
    if cmp -s "$v_path" "$m_path"; then
        echo "PASS: ${label} (byte-identical)"
        return 0
    else
        echo "FAIL: ${label}: byte mismatch between ${v_path} and ${m_path}"
        return 1
    fi
}

# superproject 不在场 (子模块独立检出) → 无 meta 副本可比, 跳过并 exit 0 (非失败)。
if [ ! -d "${META_DIR}" ]; then
    echo "SKIP: superproject not present (${SD_TOPDIR}/meta-semidrive/scripts); no canonical copies to compare"
    exit 0
fi

rc=0
check_one "tools/bpttool" "bpttool" "bpttool" || rc=1
check_one "tools/avbtool" "sign_tool/avbtool" "avbtool" || rc=1
check_one "tools/sign_tool/remote_sign_helper_with_files" "sign_tool/remote_sign_helper_with_files" "remote_sign_helper_with_files" || rc=1

# NOTE: BPT/image_cfg.h drift detection intentionally removed —
# the main repo build script (replace_image_cfg_in_ssdk) auto-syncs these
# files on every ssdk build, making post-build drift checks redundant.

if [ "$rc" -eq 0 ]; then
    echo "check_vendored_tools: ALL PASS"
else
    echo "check_vendored_tools: MISMATCH DETECTED — sync the vendored copies with meta-semidrive"
fi
exit $rc
