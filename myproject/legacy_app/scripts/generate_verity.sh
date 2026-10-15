#!/bin/sh
# Generate dm-verity hashtree for ext4 image (pure Python, no veritysetup)
# Usage: generate_verity.sh --image <img> --output-dir <dir> --block-size <size>

set -e

IMAGE=""
OUTPUT_DIR=""
BLOCK_SIZE=4096

while [ $# -gt 0 ]; do
    case "$1" in
        --image)       IMAGE="$2"; shift 2 ;;
        --output-dir)  OUTPUT_DIR="$2"; shift 2 ;;
        --block-size)  BLOCK_SIZE="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [ -z "$IMAGE" ] || [ -z "$OUTPUT_DIR" ]; then
    echo "ERROR: Missing required parameters"
    exit 1
fi

if [ ! -f "$IMAGE" ]; then
    echo "ERROR: Image not found: $IMAGE"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

# Calculate data size (file size rounded down to block boundary)
DATA_SIZE=$(stat -c%s "$IMAGE")
DATA_BLOCKS=$((DATA_SIZE / BLOCK_SIZE))

echo "  Image:      $IMAGE"
echo "  Data size:  $DATA_SIZE bytes ($DATA_BLOCKS blocks of $BLOCK_SIZE)"

_PY_SCRIPT="${OUTPUT_DIR}/_gen_verity.py"
cat > "$_PY_SCRIPT" << 'PYEOF'
import hashlib, struct, os, sys

image_path = sys.argv[1]
output_dir = sys.argv[2]
block_size = int(sys.argv[3])
data_size = int(sys.argv[4])
data_blocks = int(sys.argv[5])
hash_size = 32
hashes_per_block = block_size // hash_size

def sha256(data):
    return hashlib.sha256(data).digest()

salt = os.urandom(32)
salt_hex = salt.hex()

current = []
with open(image_path, 'rb') as f:
    for i in range(data_blocks):
        block = f.read(block_size)
        if len(block) < block_size:
            block = block + b'\x00' * (block_size - len(block))
        current.append(sha256(salt + block))

all_levels = []
while len(current) > 1:
    remainder = len(current) % hashes_per_block
    if remainder:
        current = current + [b'\x00' * hash_size] * (hashes_per_block - remainder)
    next_level = []
    for i in range(0, len(current), hashes_per_block):
        chunk = b''.join(current[i:i + hashes_per_block])
        next_level.append(sha256(salt + chunk))
    level_bytes = b''.join(current)
    pad = block_size - (len(level_bytes) % block_size)
    if pad < block_size:
        level_bytes += b'\x00' * pad
    all_levels.append(level_bytes)
    current = next_level

# root_hash is NOT stored in hashtree file (veritysetup passes it separately)
# all_levels already contains all intermediate levels from the loop

tree_data = b''.join(reversed(all_levels))
root_hash = current[0].hex()

hashtree_path = os.path.join(output_dir, 'app_hashtree.bin')
with open(hashtree_path, 'wb') as f:
    f.write(tree_data)

info_path = os.path.join(output_dir, 'verity_info.txt')
with open(info_path, 'w') as f:
    f.write('Root hash: ' + root_hash + '\n')
    f.write('Salt: ' + salt_hex + '\n')
    f.write('Hash blocks: ' + str(len(tree_data) // block_size) + '\n')

print('  Root hash:  ' + root_hash)
print('  Salt:       ' + salt_hex)
print('  Hash tree:  ' + hashtree_path + ' (' + str(len(tree_data)) + ' bytes)')
print('  Hash offset: ' + str(data_size))
PYEOF

python3 "$_PY_SCRIPT" "$IMAGE" "$OUTPUT_DIR" "$BLOCK_SIZE" "$DATA_SIZE" "$DATA_BLOCKS"
rm -f "$_PY_SCRIPT"

# Read root hash and salt for environment export
ROOT_HASH=$(grep '^Root hash:' "${OUTPUT_DIR}/verity_info.txt" | awk '{print $NF}')
SALT=$(grep '^Salt:' "${OUTPUT_DIR}/verity_info.txt" | awk '{print $NF}')
HASH_OFFSET=$DATA_SIZE

if [ -z "$ROOT_HASH" ]; then
    echo "ERROR: Failed to generate verity hashtree"
    exit 1
fi

# Export for use by sign_metadata
export ROOT_HASH SALT HASH_OFFSET DATA_SIZE DATA_BLOCKS
