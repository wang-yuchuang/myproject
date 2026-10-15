#!/bin/sh
# Create ext4 filesystem image from content directory
# Usage: create_ext4.sh --input-dir <dir> --output <img> --size <size>

set -e

INPUT_DIR=""
OUTPUT=""
SIZE=""

while [ $# -gt 0 ]; do
    case "$1" in
        --input-dir) INPUT_DIR="$2"; shift 2 ;;
        --output)    OUTPUT="$2"; shift 2 ;;
        --size)      SIZE="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [ -z "$INPUT_DIR" ] || [ -z "$OUTPUT" ] || [ -z "$SIZE" ]; then
    echo "ERROR: Missing required parameters"
    exit 1
fi

# Parse size (e.g. 256M → bytes)
SIZE_BYTES=$(echo "$SIZE" | awk '
    /[0-9]+K/ {print int($0) * 1024}
    /[0-9]+M/ {print int($0) * 1024 * 1024}
    /[0-9]+G/ {print int($0) * 1024 * 1024 * 1024}
    /[0-9]+$/ {print int($0)}
')

mkdir -p "$(dirname "$OUTPUT")"

echo "  Input:      $INPUT_DIR"
echo "  Output:     $OUTPUT"
echo "  Size:       $SIZE ($SIZE_BYTES bytes)"

# Pre-truncate to exact aligned size (avoids sparse file truncation issues)
BLOCK_SIZE=4096
ALIGNED_BYTES=$(( (SIZE_BYTES / BLOCK_SIZE) * BLOCK_SIZE ))
truncate -s "$ALIGNED_BYTES" "$OUTPUT"

# Create ext4 image with 4096-byte blocks (required for dm-verity compatibility)
mke2fs -t ext4 -b 4096 -d "$INPUT_DIR" -L "APP_DATA" "$OUTPUT" \
    "$((ALIGNED_BYTES / 4096))" > /dev/null 2>&1

echo "  Created:    $OUTPUT ($(stat -c%s "$OUTPUT") bytes)"
