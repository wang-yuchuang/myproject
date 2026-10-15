#!/bin/sh
# Generate metadata binary and optionally sign it
# Usage: sign_metadata.sh --image <ext4> --hashtree <bin> --version <ver>
#        --metadata-version <ver> --key-id <id> [--sign-url <url> --sign-token <token>]
#        --output-dir <dir>

set -e

IMAGE=""
HASHTREE=""
APP_VERSION=""
METADATA_VERSION=""
KEY_ID="ota_signing"
SIGN_URL="${SIGN_URL:-}"
SIGN_TOKEN="${SIGN_TOKEN:-${REMOTE_SIGN_TOKEN:-token-d9-sdk-001}}"
OUTPUT_DIR=""

shell_self="$0"
shell_self_dir="$(cd "$(dirname "$shell_self")" && pwd)"
CONFIG_FILE="${shell_self_dir}/../config/verity.conf"
while read -r line; do eval "$line" 2>/dev/null; done < "$CONFIG_FILE"

while [ $# -gt 0 ]; do
    case "$1" in
        --image)            IMAGE="$2"; shift 2 ;;
        --hashtree)         HASHTREE="$2"; shift 2 ;;
        --version)          APP_VERSION="$2"; shift 2 ;;
        --metadata-version) METADATA_VERSION="$2"; shift 2 ;;
        --key-id)           KEY_ID="$2"; shift 2 ;;
        --sign-url)         SIGN_URL="$2"; shift 2 ;;
        --sign-token)       SIGN_TOKEN="$2"; shift 2 ;;
        --output-dir)       OUTPUT_DIR="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [ -z "$IMAGE" ] || [ -z "$HASHTREE" ] || [ -z "$APP_VERSION" ] || [ -z "$METADATA_VERSION" ] || [ -z "$OUTPUT_DIR" ]; then
    echo "ERROR: Missing required parameters"
    exit 1
fi

# Read verity parameters from environment or verity_info.txt
ROOT_HASH="${ROOT_HASH:-$(grep "^Root hash:" "$OUTPUT_DIR/verity_info.txt" | awk '{print $NF}')}"
SALT="${SALT:-$(grep "^Salt:" "$OUTPUT_DIR/verity_info.txt" | awk '{print $NF}')}"
DATA_SIZE=$(stat -c%s "$IMAGE")
HASHTREE_SIZE=$(stat -c%s "$HASHTREE")
HASH_OFFSET=$DATA_SIZE
DATA_BLOCKS=$((DATA_SIZE / DATA_BLOCK_SIZE))

mkdir -p "$OUTPUT_DIR"

echo "  Version:    $APP_VERSION"
echo "  Metadata v: $METADATA_VERSION"
echo "  Key ID:     $KEY_ID"

# Build metadata binary
# Format: magic(8) version(4) total_len(4) header_len(4)
#         image_size(8) hash_tree_offset(8) hash_tree_size(8)
#         data_block_size(4) hash_block_size(4) data_blocks(8)
#         root_hash_len(4) salt_len(4)
#         root_hash(v) salt(v)
#         app_version(32) rollback_index(8)
#         key_id(32) signature_alg(16) signature_len(4)
#         padding + signature(v)

METADATA_BIN="${OUTPUT_DIR}/metadata.bin"
METADATA_SIG="${OUTPUT_DIR}/metadata.sig"
TEMP_DIR="${OUTPUT_DIR}/tmp"
mkdir -p "$TEMP_DIR"

# Create metadata binary with Python for portability
python3 -c "
import struct, os

# Fixed fields
magic = b'D9APPMD1'
version = int('$METADATA_VERSION')
key_id = b'$KEY_ID'.ljust(32, b'\x00')[:32]
app_version = b'$APP_VERSION'.ljust(32, b'\x00')[:32]
signature_alg = b'RSA-SHA256'.ljust(16, b'\x00')[:16]
root_hash_bytes = bytes.fromhex('$ROOT_HASH') if '$ROOT_HASH' else b'\x00' * 32
salt_bytes = bytes.fromhex('$SALT') if '$SALT' else b'\x00' * 32

# Sizes
image_size = $DATA_SIZE
hash_tree_offset = $HASH_OFFSET
hash_tree_size = $HASHTREE_SIZE
data_block_size = $DATA_BLOCK_SIZE
hash_block_size = $HASH_BLOCK_SIZE
data_blocks = $DATA_BLOCKS
rollback_index = 0

header = struct.pack('<8sIIIQQQIIQII',
    magic,
    version,
    0,  # total_len (filled later)
    0,  # header_len (filled later)
    image_size,
    hash_tree_offset,
    hash_tree_size,
    data_block_size,
    hash_block_size,
    data_blocks,
    len(root_hash_bytes),
    len(salt_bytes),
)

body = root_hash_bytes + salt_bytes
extra = struct.pack('<32sQ32s16sI',
    app_version,
    rollback_index,
    key_id,
    signature_alg,
    256,  # signature_len: RSA-2048=256, RSA-4096=512
)

header_len = len(header)
total_len = len(header) + len(body) + len(extra)
header = struct.pack('<8sIIIQQQIIQII',
    magic,
    version,
    total_len,
    header_len,
    image_size,
    hash_tree_offset,
    hash_tree_size,
    data_block_size,
    hash_block_size,
    data_blocks,
    len(root_hash_bytes),
    len(salt_bytes),
)

metadata = header + body + extra
with open('$METADATA_BIN', 'wb') as f:
    f.write(metadata)

print(f'  Metadata:   $METADATA_BIN ({len(metadata)} bytes)')
"

# Stage I: Sign with SECURE_KEYS_URL if provided
# Stage II: Sign via online signing service
if [ -n "$SIGN_URL" ]; then
    echo "  Signing via: $SIGN_URL"

    if echo "$SIGN_URL" | grep -qE '^https?://'; then
        # Online signing service (Stage II)
        METADATA_B64=$(base64 -w0 "$METADATA_BIN")

        RESPONSE=$(curl -s -X POST "$SIGN_URL" \
            -H "Authorization: Bearer ${SIGN_TOKEN}" \
            -H "Content-Type: application/json" \
            -d "{\"project\":\"d9_sdk\",\"version\":\"${APP_VERSION}\",\"key_id\":\"${KEY_ID}\",\"metadata\":\"${METADATA_B64}\"}" 2>&1)

        SIGNATURE=$(echo "$RESPONSE" | python3 -c "import sys,json; print(json.load(sys.stdin).get('signature',''))" 2>/dev/null)

        if [ -n "$SIGNATURE" ]; then
            echo "$SIGNATURE" | base64 -d > "$METADATA_SIG"
            echo "  Signature:   $METADATA_SIG ($(stat -c%s "$METADATA_SIG") bytes)"
        else
            echo "ERROR: Signing service returned no signature"
            echo "Response: $RESPONSE"
            exit 1
        fi
    else
        # Local key download (SECURE_KEYS_URL tar.gz - Stage I)
        echo "  Downloading keys from: $SIGN_URL"
        KEY_TMP="${TEMP_DIR}/keys"
        mkdir -p "$KEY_TMP"
        curl -s "$SIGN_URL" | tar xz -C "$KEY_TMP" 2>/dev/null

        # Find the signing key
        SIGN_KEY=$(find "$KEY_TMP" -name "${KEY_ID}.key" -o -name "trust_anchor.key" | head -1)
        if [ -z "$SIGN_KEY" ]; then
            echo "ERROR: Signing key not found in downloaded archive"
            exit 1
        fi

        openssl dgst -sha256 -sign "$SIGN_KEY" -out "$METADATA_SIG" "$METADATA_BIN"
        echo "  Signature:   $METADATA_SIG ($(stat -c%s "$METADATA_SIG") bytes)"
fi

# No SIGN_URL - unsigned
else
    echo "  WARNING: No sign-url provided, metadata will not be signed"
    touch "$METADATA_SIG"
fi
