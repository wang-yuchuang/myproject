#!/bin/bash
#
# ota-bundle.sh - SSDK SEC firmware OTA bundle builder
# Completely decoupled from meta-semidrive.
#
# Optional env vars:
#   REMOTE_SIGN_URL    (default: http://192.168.34.207:8900)
#   REMOTE_SIGN_TOKEN  (default: set in script)
#   RAUC_COMPATIBLE    (default: bmu02_a00)
#
# Optional env vars:
#   SSDK_BOARD         (default: bmu05_a00)
#   SSDK_BUILD_TYPE    (default: secure)
#   SSDK_OUTPUT_DIR    (overrides auto-detect from .project.conf)
#   SSDK_PART_SIZE     (ssystem partition size; auto-derived from BPT via bpttool)
#   SSDK_BOOT_MODE     (emmc|xip; default derived from SSDK_PROJECT)
#   VBMETA_SIGNED_ALG  (default: SHA256_RSA2048)
#
set -e

SSDK_DIR="${SSDK_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
SSDK_BOARD="${SSDK_BOARD:-bmu05_a00}"
SSDK_BUILD_TYPE="${SSDK_BUILD_TYPE:-secure}"
SSDK_PROJECT="${SSDK_PROJECT:-ref}"
# [ssdk-standalone-boot-mode] 启动模式: 显式 env 优先, 否则从 SSDK_PROJECT 推导
# (xip/xip_b → xip; 其余 → emmc), 保持 legacy -p (project) 行为。
# [ssdk-standalone-boot-mode] 启动模式: 显式 env 优先 (-m 传入 SSDK_BOOT_MODE);
# legacy (未显式) 从 SSDK_PROJECT 推导, 但仅当 slot B 产物存在才判 xip,
# 否则回退 legacy 单镜像 —— 保持 legacy -p 行为不回归 (D1 兼容承诺)。
if [ -z "${SSDK_BOOT_MODE:-}" ]; then
    case "${SSDK_PROJECT}" in
        xip|xip_b)
            if [ -f "${SSDK_DIR}/boards/${SSDK_BOARD}/build/xip_b/gcc/${SSDK_BUILD_TYPE}/secure.bin" ]; then
                SSDK_BOOT_MODE="xip"
            else
                SSDK_BOOT_MODE="emmc"
            fi
            ;;
        *) SSDK_BOOT_MODE="emmc" ;;
    esac
fi
SSYSTEM_IMG="${SSDK_DIR}/boards/${SSDK_BOARD}/build/${SSDK_PROJECT}/gcc/${SSDK_BUILD_TYPE}/secure.bin"
# SSDK_PART_SIZE / SSDK_VBMETA_PART_SIZE 由 BPT (bpttool) 推导, 见 ssdk_derive_part_sizes()
# (不再硬编码默认 8388608; SSDK_PART_SIZE env 仍可显式覆盖 ssystem)
SD_TOPDIR="${SD_TOPDIR:-$(cd "$SSDK_DIR/../.." && pwd)}"

DEPLOY_DIR=""
if [ -f "${SD_TOPDIR}/.project.conf" ]; then
    source "${SD_TOPDIR}/.project.conf" 2>/dev/null || true
    DEPLOY_DIR="${SD_TOPDIR}/out/${MACHINE_DEPLOYDIR}"
fi
DEPLOY_DIR="${SSDK_OUTPUT_DIR:-${DEPLOY_DIR}}"
OTA_OUTPUT="${DEPLOY_DIR}/ota"
mkdir -p "$OTA_OUTPUT"

SIGN_SERVER="${REMOTE_SIGN_URL:-http://192.168.34.207:8900}"
SIGN_TOKEN="${REMOTE_SIGN_TOKEN:-token-d9-sdk-001}"
# 同硬件双形态统一 compatible。standalone 以 -b 指定板为准 —— 不依赖可能陈旧的
# .project.conf MACHINE_PROJECT, 否则会产出板卡不匹配的不可安装 bundle。
# 优先: 显式 env RAUC_COMPATIBLE > 本板 project.conf 的 SD_RAUC_COMPATIBLE > SSDK_BOARD。
ssdk_board_conf="${SD_TOPDIR}/source/cfg_industry/d9360/projects/${SSDK_BOARD}/project.conf"
ssdk_board_compat=""
if [ -f "$ssdk_board_conf" ]; then
    ssdk_board_compat=$(sed -n 's/^SD_RAUC_COMPATIBLE=//p' "$ssdk_board_conf" 2>/dev/null | tr -d '"' | head -n1)
fi
RAUC_COMPATIBLE="${RAUC_COMPATIBLE:-${ssdk_board_compat:-${SSDK_BOARD}}}"
KEY_ID="ota_signing"
VBMETA_SIGNED_ALG="${VBMETA_SIGNED_ALG:-SHA256_RSA2048}"

RAUC_BIN="rauc"
[ -n "${SD_BUILD_OBJDIR}" ] && [ -x "${SD_BUILD_OBJDIR}/ap1_${MACHINE_PROJECT}_buildroot/host/bin/rauc" ] && \
    RAUC_BIN="${SD_BUILD_OBJDIR}/ap1_${MACHINE_PROJECT}_buildroot/host/bin/rauc"

# Tool paths (ssdk-local 唯一解析; [ssdk-standalone-boot-mode] 不再回退 meta)
# PYTHONPATH: bpttool 依赖 sfs_module (在 tools/sdtools/pactool/)。顶层 meta 委托调用时无 build.sh
# 的 PYTHONPATH 导出，必须在此自设 (5.6 回归发现的缺失)。
PYTHONPATH="${PYTHONPATH}:${SSDK_DIR}/tools/sdtools/pactool"
export PYTHONPATH
AVBTOOL="${SSDK_DIR}/tools/avbtool"
SIGN_HELPER="${SSDK_DIR}/tools/sign_tool/remote_sign_helper_with_files"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[0;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[SSDK-OTA]${NC} $*"; }
warn()  { echo -e "${YELLOW}[SSDK-OTA] WARNING:${NC} $*" >&2; }
error() { echo -e "${RED}[SSDK-OTA]${NC} $*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# [ssdk-standalone-boot-mode] 分区大小推导 (BPT via bpttool; 不硬编码)
#   SSDK_PART_SIZE:         ssystem 分区大小 (env 可覆盖)
#   SSDK_VBMETA_PART_SIZE:  vbmeta-ssdk 分区大小 (始终从 BPT 推导)
#   BPT 选择: xip → ospi2.bpt; emmc → global_emmc_only.bpt
# ---------------------------------------------------------------------------
ssdk_query_part_size() {
    local label="$1" bpt_file="$2"
    local bpttool="${SSDK_BPTTOOL:-${SSDK_DIR}/tools/bpttool}"
    local tmp_bpt size
    [ -f "$bpt_file" ] || return 1
    [ -f "$bpttool" ] || return 1
    # 与 scripts/calc_xip_membase.sh 一致: 先 make_table 展开尺寸/偏移, 再 query_partition
    tmp_bpt=$(mktemp /tmp/ssdk_ota_bpt_XXXXXX.json)
    if ! python "${bpttool}" make_table \
        --input "${bpt_file}" \
        --output_json "${tmp_bpt}" 2>/dev/null; then
        rm -f "$tmp_bpt"
        return 1
    fi
    size=$(python "${bpttool}" query_partition \
        --input "${tmp_bpt}" \
        --label "${label}" \
        --ab_collapse \
        --type size 2>/dev/null)
    rm -f "$tmp_bpt"
    if [ -z "$size" ] || [ "$size" = "None" ]; then
        return 1
    fi
    echo "$size"
}

ssdk_derive_part_sizes() {
    local bpt_file bpt_name
    if [ "${SSDK_BOOT_MODE}" = "xip" ]; then
        bpt_name="ospi2.bpt"
    else
        bpt_name="global_emmc_only.bpt"
    fi
    # BPT: SSDK_BPT_DIR env (最高) → cfg_industry (superproject) → ssdk-local copy → fail-fast
    local _ssdk_bpt="${SSDK_DIR}/boards/${SSDK_BOARD}/secure/pack_cfg/${bpt_name}"
    local _cfg_bpt="${SD_TOPDIR}/source/cfg_industry/d9360/projects/${SSDK_BOARD}/pack_cfg/${bpt_name}"
    if [ -n "${SSDK_BPT_DIR:-}" ]; then
        bpt_file="${SSDK_BPT_DIR}/${bpt_name}"
    elif [ -f "${_cfg_bpt}" ]; then
        bpt_file="${_cfg_bpt}"
    elif [ -f "${_ssdk_bpt}" ]; then
        bpt_file="${_ssdk_bpt}"
    else
        error "BPT not found: tried ${_cfg_bpt} and ${_ssdk_bpt}; set SSDK_BPT_DIR"
    fi
    if [ ! -f "${bpt_file}" ]; then
        error "BPT not found: ${bpt_file} (set SSDK_BPT_DIR to <pack_cfg dir>)"
    fi

    # ssystem: env 覆盖优先, 否则从 BPT 推导
    if [ -z "${SSDK_PART_SIZE:-}" ]; then
        SSDK_PART_SIZE=$(ssdk_query_part_size "ssystem" "$bpt_file" || true)
        if [ -z "$SSDK_PART_SIZE" ]; then
            error "cannot derive ssystem partition size from BPT (${bpt_file}); set SSDK_PART_SIZE explicitly"
        fi
    fi

    # vbmeta-ssdk: 始终从 BPT 推导 (SSDK_PART_SIZE 覆盖不影响它)
    SSDK_VBMETA_PART_SIZE=$(ssdk_query_part_size "vbmeta-ssdk" "$bpt_file" || true)
    if [ -z "$SSDK_VBMETA_PART_SIZE" ]; then
        error "cannot derive vbmeta-ssdk partition size from BPT (${bpt_file})"
    fi
}

# ---------------------------------------------------------------------------
# 版本号 (基于SSDK仓库；统一权威：superproject meta-semidrive/scripts/auto_version.sh)
# ---------------------------------------------------------------------------
# [ssdk-standalone-boot-mode] 逐字等价四态版本生成器 (superproject meta-semidrive/scripts/auto_version.sh 的内联回退)。
# 仅当 superproject 脚本不可达 (子模块独立构建) 时使用; 输出必须与 auto_version.sh plain 格式逐字节一致。
# 返回码: 0 成功; 2 prefix/length 断言失败。
ssdk_inline_auto_version() {
    local dir="$1" prefix="$2"
    local full_prefix="" body="" sha8="" tag="" describe="" is_git=false dirty=false

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


ssdk_version() {
    # 前缀：优先 BSP_PREFIX（project_build_link.sh 已导出），否则 D9360-<SSDK_BOARD>（standalone 回退）
    local prefix="${BSP_PREFIX:-D9360-${SSDK_BOARD}}"

    # 从脚本位置稳健定位 superproject 的 auto_version.sh：
    #   SSDK_DIR = <superproject>/source/ssdk（子模块根），脚本在 <SSDK_DIR>/../../meta-semidrive/scripts/auto_version.sh
    local av_script="${SSDK_DIR}/../../meta-semidrive/scripts/auto_version.sh"
    local toplevel
    if toplevel="$(git -C "$SSDK_DIR" rev-parse --show-toplevel 2>/dev/null)"; then
        av_script="${toplevel}/../../meta-semidrive/scripts/auto_version.sh"
    fi

    if [ -x "$av_script" ]; then
        "$av_script" --dir "$SSDK_DIR" --prefix "$prefix"
    else
        # 子模块独立构建: 逐字等价内联回退
        ssdk_inline_auto_version "$SSDK_DIR" "$prefix" || error "auto_version.sh not found ($av_script) and inline fallback failed"
    fi
}

# ---------------------------------------------------------------------------
# 从远程签名服务器下载证书并提取公钥
# 参考: emmc_pac_separate.sh §222-253
# ---------------------------------------------------------------------------
ssdk_fetch_signing_key() {
    local TMPDIR="$1"
    info "Fetching signing key from ${SIGN_SERVER}..."
    
    # 下载远程证书（base64 编码的 DER 证书链）
    local _REMOTE_CERT_RAW
    _REMOTE_CERT_RAW=$(curl -s --max-time 30 "${SIGN_SERVER}/api/v1/cert/vbmeta_signing")
    
    if [ -z "$_REMOTE_CERT_RAW" ]; then
        error "Failed to download certificate from remote signing server"
    fi
    
    # base64 解码为 DER 证书链
    echo "$_REMOTE_CERT_RAW" | base64 -d > "${TMPDIR}/remote_vbmeta_cert_chain.der"
    
    # 保存证书链供 --public_key_meta 使用
    local KEY_META="${TMPDIR}/remote_vbmeta.cer.chain"
    cp "${TMPDIR}/remote_vbmeta_cert_chain.der" "$KEY_META"
    
    # 从第一个证书提取公钥 PEM
    openssl x509 -in "${TMPDIR}/remote_vbmeta_cert_chain.der" -inform DER -noout -pubkey \
        > "${TMPDIR}/remote_vbmeta_pub.pem" 2>/dev/null
    
    if [ ! -f "${TMPDIR}/remote_vbmeta_pub.pem" ] || [ ! -s "${TMPDIR}/remote_vbmeta_pub.pem" ]; then
        error "Failed to extract public key from certificate"
    fi
    
    info "Remote public key and cert chain ready"
}

# ---------------------------------------------------------------------------
# 生成 vbmeta-ssdk.img
# 完全参考: emmc_pac_separate.sh §369-381, sign_helper.sh image_add_hash_footer
# [ssdk-standalone-boot-mode] 签名泛化: 显式 out_name / part_size
#   用法: ssdk_gen_vbmeta_ssdk <sdk_img> <out_dir> <tmp_dir> <out_name> <part_size>
# ---------------------------------------------------------------------------
ssdk_gen_vbmeta_ssdk() {
    local sdk_img="$1"     # ssystem.img path
    local out_dir="$2"     # output directory for vbmeta-ssdk.img
    local tmp_dir="$3"     # temp directory (already has key files)
    local out_name="$4"    # output filename (vbmeta-ssdk.img / vbmeta-ssdk_a.img / ...)
    local part_size="$5"   # ssystem partition size (bytes) for hash footer
    local ATB_SIGN_KEY="${tmp_dir}/remote_vbmeta_pub.pem"
    local KEY_META="${tmp_dir}/remote_vbmeta.cer.chain"
    
    # Step 1: add_hash_footer → 生成 .vbmeta.blob (包含 hash descriptor)
    # 使用临时副本，不修改源文件
    local VBMETA_BLOB="${tmp_dir}/${out_name}.vbmeta.blob"
    local TEMP_IMG="${tmp_dir}/${out_name}.hash_footer_temp"
    
    info "Adding hash footer to ${out_name}..."
    cp -f "$sdk_img" "$TEMP_IMG"
    
    ${AVBTOOL} add_hash_footer \
        --partition_name ssystem \
        --partition_size ${part_size} \
        --image ${TEMP_IMG} \
        --compact_footer \
        --output_vbmeta_image ${VBMETA_BLOB} \
        --do_not_append_vbmeta_image \
        --hash_algorithm sha256
    
    # Step 2: make_vbmeta_image → 生成 vbmeta-ssdk.img
    local SSDK_DESCRIPTOR=" --include_descriptors_from_image ${VBMETA_BLOB} "
    local VBMETA_SIGNED_ARGS="--algorithm ${VBMETA_SIGNED_ALG} --key ${ATB_SIGN_KEY} --signing_helper_with_files ${SIGN_HELPER} --public_key_meta ${KEY_META}"
    
    info "Generating ${out_name}..."
    ${AVBTOOL} make_vbmeta_image \
        --output "${out_dir}/${out_name}" \
        ${VBMETA_SIGNED_ARGS} \
        --rollback_index 0 \
        --prop rollback_index.location.override:31 \
        ${SSDK_DESCRIPTOR} || error "${out_name} generation failed"
    
    # 尺寸断言: vbmeta 必须 ≤ vbmeta-ssdk 分区 (hard-fail)
    local vb_size=$(stat -c%s "${out_dir}/${out_name}" 2>/dev/null || echo 0)
    if [ -n "${SSDK_VBMETA_PART_SIZE}" ] && [ "$vb_size" -gt "${SSDK_VBMETA_PART_SIZE}" ]; then
        error "${out_name} (${vb_size} bytes) exceeds vbmeta-ssdk partition (${SSDK_VBMETA_PART_SIZE} bytes)"
    fi
    info "${out_name} generated (${vb_size} bytes)"
}

# ---------------------------------------------------------------------------
# ssystem 尺寸断言: 必须 ≤ SSDK_PART_SIZE (hard-fail);
# 超过 512 KiB (524288 bytes, FDA memsize hint) 仅 WARNING (non-fatal)。
# ---------------------------------------------------------------------------
ssdk_assert_ssystem_size() {
    local img="$1"
    local sz=$(stat -c%s "$img" 2>/dev/null || echo 0)
    if [ -n "${SSDK_PART_SIZE}" ] && [ "$sz" -gt "${SSDK_PART_SIZE}" ]; then
        error "ssystem image ${img} (${sz} bytes) exceeds ssystem partition (${SSDK_PART_SIZE} bytes)"
    fi
    # 512KiB 仅适用 emmc (FDA memsize hint); XIP 走 PT_LD_XIP_SKIP, FDA 不拷贝 ssystem, 故不告警。
    if [ "$sz" -gt 524288 ] && [ "${SSDK_BOOT_MODE}" != "xip" ]; then
        warn "ssystem image ${img} (${sz} bytes) exceeds 512 KiB (524288 bytes) FDA memsize hint"
    fi
}

# ---------------------------------------------------------------------------
# 生成 manifest.raucm (emmc 模式: [image.ssystem] + [image.vbmeta-ssdk])
# ---------------------------------------------------------------------------
gen_manifest() {
    cat > "$2" << MANIFEST_EOF
[update]
compatible=${RAUC_COMPATIBLE}
version=${1}
description=SEC firmware update (SSDK)

[bundle]
format=verity

[image.ssystem]
filename=ssystem.img

[image.vbmeta-ssdk]
filename=vbmeta-ssdk.img
MANIFEST_EOF
}

# ---------------------------------------------------------------------------
# 生成 XIP manifest.raucm (单 [image.ssystem] 指向 stub; 4 个真镜像为 loose payload)
# eMMC vbmeta-ssdk 占位仅 8KB, 容不下 64KB 真镜像, 故 manifest 不含 vbmeta-ssdk image。
# stub 内容必须跨 bundle 唯一 (version + 时间戳), 否则 RAUC "skip correct image"
# 不推进 timestamp, 导致后续 target 卡死。
# ---------------------------------------------------------------------------
gen_xip_manifest() {
    { echo "XIP ssystem OTA stub"; echo "version=${1}"; date +"built=%F %T"; } > "${3}/ssystem_stub.img"
    cat > "$2" << MANIFEST_EOF
[update]
compatible=${RAUC_COMPATIBLE}
version=${1}
description=SEC XIP firmware update (NOR) for ${RAUC_COMPATIBLE}

[bundle]
format=verity

[image.ssystem]
filename=ssystem_stub.img
MANIFEST_EOF
}

# ---------------------------------------------------------------------------
# RAUC CMS 远程签名 + 打包
# ---------------------------------------------------------------------------
sign_bundle() {
    local d="$1" o="$2"
    local t=$(mktemp -d /tmp/ssdk_ota_sign_XXXXXX)
    trap 'trap - RETURN; if [ -n "${t:-}" ]; then shred -u "${t}/temp.key" 2>/dev/null || true; rm -rf "${t}" || true; fi' RETURN
    info "Using RAUC: ${RAUC_BIN}"
    info "Generating temp key..."
    openssl req -x509 -newkey rsa:2048 -keyout "${t}/temp.key" -out "${t}/temp.crt" -days 1 -nodes -subj "/CN=ssdk" 2>/dev/null
    info "Creating bundle..."
    "${RAUC_BIN}" bundle --cert="${t}/temp.crt" --key="${t}/temp.key" "${d}" "${t}/temp.raucb" || error "Bundle creation failed"
    info "Extracting CMS..."
    "${RAUC_BIN}" extract-signature --keyring="${t}/temp.crt" "${t}/temp.raucb" "${t}/temp.cms" 2>/dev/null
    openssl cms -resign -nocerts -inform DER -in "${t}/temp.cms" -signer "${t}/temp.crt" -inkey "${t}/temp.key" -outform DER -out "${t}/stripped.cms" || error "CMS strip failed"
    info "Remote signing..."
    local retry=0 http=""
    while [ $retry -lt 3 ]; do
        http=$(curl -s -w "%{http_code}" -o "${t}/sig.cms" -X POST -H "Authorization: Bearer ${SIGN_TOKEN}" --data-binary "@${t}/stripped.cms" "${SIGN_SERVER}/api/v1/sign/rauc/cms?project=d9_sdk&key_id=${KEY_ID}" 2>/dev/null)
        [ "$http" = "200" ] && break
        retry=$((retry+1)); [ $retry -lt 3 ] && sleep $retry
    done
    [ "$http" != "200" ] && error "Remote signing failed (HTTP $http)"
    info "Replacing signature..."
    rm -f "$o"
    "${RAUC_BIN}" replace-signature --no-verify "${t}/temp.raucb" "${t}/sig.cms" "$o" || error "Replace-signature failed"
    info "Done: $o ($(stat -c%s "$o" 2>/dev/null || echo 0) bytes)"
}

# ---------------------------------------------------------------------------
# 主流程: 构建 ssystem OTA bundle (含 vbmeta-ssdk)
#   xip : 双槽 A/B loose payload (ssystem_a/b.img + vbmeta-ssdk_a/b.img) + 单 [image.ssystem] stub
#   emmc: 单镜像 (ssystem.img + vbmeta-ssdk.img), RAUC 原生写 eMMC
# ---------------------------------------------------------------------------
build() {
    # 显式跳过打包 (design D4 escape): 仅编译不产 bundle (顶层/standalone 均可)。
    if [ "${SSDK_SKIP_BUNDLE:-0}" = "1" ]; then
        info "SSDK_SKIP_BUNDLE=1: skipping ssystem bundle build (compile-only)"
        return 0
    fi
    local v=$(ssdk_version)
    info "Building SSDK OTA bundle"
    info "  Board: $SSDK_BOARD  Type: $SSDK_BUILD_TYPE  Boot mode: $SSDK_BOOT_MODE  Version: $v"

    # 0. 推导 ssystem / vbmeta-ssdk 分区大小 (BPT via bpttool)
    ssdk_derive_part_sizes
    info "  ssystem partition: ${SSDK_PART_SIZE} bytes  vbmeta-ssdk partition: ${SSDK_VBMETA_PART_SIZE} bytes"

    # 创建临时工作目录
    local d=$(mktemp -d)
    trap "rm -rf $d" EXIT

    # 1. 获取远程签名密钥
    mkdir -p "${d}/keys"
    ssdk_fetch_signing_key "${d}/keys"

    # 远程签名环境 (REMOTE_SIGN_KEY_ID=vbmeta_signing 供 avbtool signing helper 使用)
    export REMOTE_SIGN_URL="${SIGN_SERVER}"
    export REMOTE_SIGN_TOKEN="${SIGN_TOKEN}"
    export REMOTE_SIGN_KEY_ID=vbmeta_signing

    if [ "${SSDK_BOOT_MODE}" = "xip" ]; then
        # --- XIP: 双槽 (A/B) loose payload + 单 [image.ssystem] stub ---
        local slot_a="${SSDK_DIR}/boards/${SSDK_BOARD}/build/xip/gcc/${SSDK_BUILD_TYPE}/secure.bin"
        local slot_b="${SSDK_DIR}/boards/${SSDK_BOARD}/build/xip_b/gcc/${SSDK_BUILD_TYPE}/secure.bin"
        [ -f "$slot_a" ] || error "Slot A firmware not found: $slot_a (build SSDK with -m xip first)"
        [ -f "$slot_b" ] || error "Slot B firmware not found: $slot_b (build SSDK with -m xip first)"

        # A/B 固件能力门控 (design D4): 两槽变体均须启用 GPT_SLOT_SWITCH 或 SYSMON_BOOT_MEDIA_NOR_XIP。
        # 检测源 = 本次构建生成的 per-slot 变体 defconfig (out/variant_defconfig/...), 非共享 ref/.config。
        local _vd_a="${SSDK_DIR}/out/variant_defconfig/${SSDK_BOARD}/xip/defconfig"
        local _vd_b="${SSDK_DIR}/out/variant_defconfig/${SSDK_BOARD}/xip_b/defconfig"
        if ! { [ -f "$_vd_a" ] && grep -qE '^CONFIG_(GPT_SLOT_SWITCH|SYSMON_BOOT_MEDIA_NOR_XIP)=y' "$_vd_a"; } \
           || ! { [ -f "$_vd_b" ] && grep -qE '^CONFIG_(GPT_SLOT_SWITCH|SYSMON_BOOT_MEDIA_NOR_XIP)=y' "$_vd_b"; }; then
            error "firmware lacks A/B slot-switch capability: need CONFIG_GPT_SLOT_SWITCH=y or CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP=y in BOTH slot variant defconfigs (${_vd_a} / ${_vd_b}); rebuild with -m xip, or set SSDK_SKIP_BUNDLE=1 to compile without bundle"
        fi

        info "Copying ssystem_a.img / ssystem_b.img..."
        cp -L "$slot_a" "${d}/ssystem_a.img"
        cp -L "$slot_b" "${d}/ssystem_b.img"

        # 2. 生成两个槽的 vbmeta-ssdk (descriptor partition_name 均 = ssystem)
        ssdk_gen_vbmeta_ssdk "${d}/ssystem_a.img" "$d" "${d}/keys" "vbmeta-ssdk_a.img" "${SSDK_PART_SIZE}"
        ssdk_gen_vbmeta_ssdk "${d}/ssystem_b.img" "$d" "${d}/keys" "vbmeta-ssdk_b.img" "${SSDK_PART_SIZE}"

        # 3. ssystem 尺寸断言 (hard-fail) + FDA memsize 提示 (non-fatal)
        ssdk_assert_ssystem_size "${d}/ssystem_a.img"
        ssdk_assert_ssystem_size "${d}/ssystem_b.img"

        # 4. 生成 XIP manifest + stub
        info "Generating XIP manifest..."
        gen_xip_manifest "$v" "${d}/manifest.raucm" "$d"
    else
        # --- emmc: 单镜像 + 传统 manifest (RAUC 原生写 eMMC) ---
        [ ! -f "$SSYSTEM_IMG" ] && error "Firmware not found: $SSYSTEM_IMG (build SSDK first)"

        # 2. 复制 ssystem 固件
        info "Copying ssystem.img..."
        cp -L "$SSYSTEM_IMG" "${d}/ssystem.img"

        # 3. 生成 vbmeta-ssdk.img (与 emmc_pac_separate.sh 完全一致的流程)
        ssdk_gen_vbmeta_ssdk "${d}/ssystem.img" "$d" "${d}/keys" "vbmeta-ssdk.img" "${SSDK_PART_SIZE}"

        # 3b. ssystem 尺寸断言 (hard-fail) + FDA memsize 提示 (non-fatal)
        ssdk_assert_ssystem_size "${d}/ssystem.img"

        # 4. 生成 manifest
        info "Generating manifest..."
        gen_manifest "$v" "${d}/manifest.raucm"
    fi

    # 5. 签名打包
    local output_file="${OTA_OUTPUT}/ota-ssystem-${v}.raucb"
    sign_bundle "$d" "$output_file"

    # 6. 记录历史
    mkdir -p "${OTA_OUTPUT}/history"
    local ts=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local sz=$(stat -c%s "$output_file" 2>/dev/null || echo 0)
    cat > "${OTA_OUTPUT}/history/ssdk-${v}.json" << HIST_EOF
{"type":"ssystem","version":"$v","timestamp":"$ts","board":"$SSDK_BOARD",
 "bundle":"ota-ssystem-${v}.raucb","size":$sz,
 "include_vbmeta_ssdk":true,
 "boot_mode":"${SSDK_BOOT_MODE}"}
HIST_EOF

    info "Build complete!"
    info "  Output: $output_file"
}

clean() { rm -f "${OTA_OUTPUT}/ota-ssystem-"*.raucb; info "Cleaned"; }

case "${1:-build}" in
    build)   build ;;
    clean)   clean ;;
    version) ssdk_version ;;
    *) echo "Usage: $0 {build|clean|version}" >&2; exit 1 ;;
esac
