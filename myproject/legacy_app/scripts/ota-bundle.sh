#!/bin/sh
# ota-bundle.sh — Independent RAUC bundle builder for app partition
# Part of source/test_app/ submodule
#
# Usage: ota-bundle.sh --app-image <path> --output <path> --version <ver>
#        --compatible <string> --rauc-cert <path> --rauc-key <path>
#        [--sign-url <url> --sign-token <token> --sign-key-id <id>]

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info() { echo -e "${GREEN}[APP-OTA]${NC} $*"; }
warn() { echo -e "${YELLOW}[APP-OTA-WARN]${NC} $*" >&2; }
error() { echo -e "${RED}[APP-OTA-ERROR]${NC} $*" >&2; exit 1; }

# Defaults
APP_IMAGE=""
OUTPUT=""
VERSION=""
COMPATIBLE=""
RAUC_CERT=""
RAUC_KEY=""
SIGN_URL="${SIGN_URL:-${REMOTE_SIGN_URL:-http://192.168.34.207:8900}}"
SIGN_TOKEN="${SIGN_TOKEN:-${REMOTE_SIGN_TOKEN:-token-d9-sdk-001}}"
SIGN_KEY_ID="ota_signing"

# Parse args
while [ $# -gt 0 ]; do
    case "$1" in
        --app-image)    APP_IMAGE="$2"; shift 2 ;;
        --output)       OUTPUT="$2"; shift 2 ;;
        --version)      VERSION="$2"; shift 2 ;;
        --compatible)   COMPATIBLE="$2"; shift 2 ;;
        --rauc-cert)    RAUC_CERT="$2"; shift 2 ;;
        --rauc-key)     RAUC_KEY="$2"; shift 2 ;;
        --sign-url)     SIGN_URL="$2"; shift 2 ;;
        --sign-token)   SIGN_TOKEN="$2"; shift 2 ;;
        --sign-key-id)  SIGN_KEY_ID="$2"; shift 2 ;;
        --help)
            echo "Usage: $0 --app-image <path> --output <path> --version <ver>"
            echo "       --compatible <string> --rauc-cert <path> --rauc-key <path>"
            echo "       [--sign-url <url> --sign-token <token> --sign-key-id <id>]"
            exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# Validate required params
[ -z "$APP_IMAGE" ] || [ -z "$OUTPUT" ] || [ -z "$VERSION" ] || [ -z "$COMPATIBLE" ] && {
    error "Missing required params: --app-image --output --version --compatible"; }
[ ! -f "$APP_IMAGE" ] && error "App image not found: $APP_IMAGE"

# Resolve rauc binary (prefer bundled tools/rauc, then Buildroot host-rauc >= 1.15.2)
resolve_rauc_bin() {
    local script_dir="$(cd "$(dirname "$0")" && pwd)"
    local sdk_top="${1:-$(cd "${script_dir}/../.." && pwd)}"

    # 1. Bundled tools/bin/rauc (self-contained, works in Docker)
    local bundled_rauc="${script_dir}/../tools/bin/rauc"
    if [ -x "$bundled_rauc" ]; then
        local bundled_lib="${script_dir}/../tools/lib"
        if [ -d "$bundled_lib" ]; then
            export LD_LIBRARY_PATH="${bundled_lib}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
        fi
        export PATH="${script_dir}/../tools/bin${PATH:+:$PATH}"
        RAUC_BIN="$bundled_rauc"
        return
    fi

    # 2. Buildroot host-rauc
    local project_conf="${sdk_top}/.project.conf"
    local build_objdir="${sdk_top}/out"
    if [ -f "$project_conf" ]; then
        source "$project_conf"
        local host_rauc="${build_objdir}/${MACHINE_DEPLOYDIR:-}/build_object/ap1_${MACHINE_PROJECT:-}_buildroot/host/bin/rauc"
        if [ -x "$host_rauc" ]; then
            RAUC_BIN="$host_rauc"
            return
        fi
    fi

    # 3. System rauc (may be too old)
    if command -v rauc >/dev/null 2>&1; then
        RAUC_BIN="rauc"
        return
    fi

    RAUC_BIN=""
}

# Build bundle with local cert/key
build_local() {
    local input_dir="$1"
    local output_file="$2"
    local cert="$3"
    local key="$4"

    info "  Signing: Local cert/key"
    [ ! -f "$cert" ] && error "RAUC cert not found: $cert"
    [ ! -f "$key" ] && error "RAUC key not found: $key"

    "$RAUC_BIN" bundle --cert="$cert" --key="$key" "${input_dir}" "${output_file}"
    [ $? -ne 0 ] && error "Bundle creation failed"
}

# Build bundle with remote CMS resigning (mirrors meta-semidrive ota_sign_and_bundle)
build_remote_cms() {
    local input_dir="$1"
    local output_file="$2"
    local sign_url="$3"
    local sign_token="$4"
    local key_id="$5"

    info "  Signing: Remote CMS (${sign_url})"

    local tmpdir
    tmpdir=$(mktemp -d /tmp/app_ota_sign_XXXXXX)
    trap "shred -u ${tmpdir}/temp.key 2>/dev/null; rm -rf ${tmpdir}" EXIT

    # 1. Generate temp key
    info "  Generating temp key..."
    openssl req -x509 -newkey rsa:2048 -keyout "${tmpdir}/temp.key" \
        -out "${tmpdir}/temp.crt" -days 1 -nodes -subj "/CN=temp" 2>/dev/null

    # 2. Create verity bundle with temp key
    info "  Creating verity bundle with temp key..."
    local bundle_tmp="${tmpdir}/temp.raucb"
    "${RAUC_BIN}" bundle --cert="${tmpdir}/temp.crt" --key="${tmpdir}/temp.key" \
        "${input_dir}" "${bundle_tmp}"
    [ $? -ne 0 ] && error "Bundle creation failed"

    # 3. Extract CMS signature
    info "  Extracting temp CMS signature..."
    "${RAUC_BIN}" extract-signature --keyring="${tmpdir}/temp.crt" "${bundle_tmp}" "${tmpdir}/temp.cms"

    # 4. Strip temp cert from CMS
    info "  Stripping temp cert from CMS..."
    openssl cms -resign -nocerts -inform DER -in "${tmpdir}/temp.cms" \
        -signer "${tmpdir}/temp.crt" -inkey "${tmpdir}/temp.key" \
        -outform DER -out "${tmpdir}/stripped.cms"
    [ $? -ne 0 ] && error "CMS cert stripping failed"

    # 5. Remote CMS resign
    info "  Remote CMS resigning..."
    local cms_blob="${tmpdir}/server-sig.cms"
    local retry=0
    local cms_http=""
    while [ $retry -lt 3 ]; do
        cms_http=$(curl -s -w "%{http_code}" -o "${cms_blob}" \
            -X POST -H "Authorization: Bearer ${sign_token}" \
            --data-binary "@${tmpdir}/stripped.cms" \
            "${sign_url}/api/v1/sign/rauc/cms?project=d9_sdk&key_id=${key_id}" 2>/dev/null)
        [ "${cms_http}" = "200" ] && break
        retry=$((retry + 1))
        [ $retry -lt 3 ] && sleep $retry && warn "Retry ${retry}/3 after ${retry}s..."
    done
    [ "${cms_http}" != "200" ] && error "Remote CMS failed after 3 retries (HTTP ${cms_http})"

    # 6. Replace signature
    info "  Replacing signature..."
    rm -f "${output_file}"
    "${RAUC_BIN}" replace-signature --no-verify "${bundle_tmp}" "${cms_blob}" "${output_file}"
    [ $? -ne 0 ] && error "Replace-signature failed"

    # Cleanup temp signing dir
    shred -u "${tmpdir}/temp.key" 2>/dev/null || true
    rm -rf "${tmpdir}"

    # Reset trap since we cleaned up
    trap - EXIT
}

# ── Main ──

info "Building app OTA bundle..."
info "  Version: $VERSION"
info "  Compatible: $COMPATIBLE"

# Resolve RAUC binary (function sets LD_LIBRARY_PATH and PATH as side-effects)
SDK_TOP="${SDK_TOP:-$(cd "$(dirname "$0")/../.." && pwd)}"
resolve_rauc_bin "$SDK_TOP"   # sets RAUC_BIN and exports LD_LIBRARY_PATH/PATH
[ -z "$RAUC_BIN" ] && error "rauc not found in PATH or Buildroot output"
info "  RAUC: $RAUC_BIN"

# Create temp build dir
BUILD_DIR=$(mktemp -d)
trap "rm -rf $BUILD_DIR" EXIT

# Copy app image
cp -L "$APP_IMAGE" "${BUILD_DIR}/app.img"
info "  App image: $(basename "$APP_IMAGE") ($(numfmt --to=iec $(stat -c%s "$APP_IMAGE") 2>/dev/null || stat -c%s "$APP_IMAGE")B)"

# Generate manifest
cat > "${BUILD_DIR}/manifest.raucm" << EOF
[update]
compatible=${COMPATIBLE}
version=${VERSION}
description=App partition update for ${COMPATIBLE}

[bundle]
format=verity

[image.app]
filename=app.img
EOF

info "  Manifest generated"

# Determine signing method and build
if [ -n "$SIGN_URL" ]; then
    build_remote_cms "$BUILD_DIR" "$OUTPUT" "$SIGN_URL" "$SIGN_TOKEN" "$SIGN_KEY_ID"
elif [ -n "$RAUC_CERT" ] && [ -n "$RAUC_KEY" ]; then
    build_local "$BUILD_DIR" "$OUTPUT" "$RAUC_CERT" "$RAUC_KEY"
else
    error "No signing method specified. Use --rauc-cert/--rauc-key or --sign-url"
fi

info "Bundle created: $OUTPUT ($(numfmt --to=iec $(stat -c%s "$OUTPUT") 2>/dev/null || stat -c%s "$OUTPUT")B)"
info "Done!"
