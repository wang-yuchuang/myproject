#!/bin/sh
# Assemble app.img: ext4 + hashtree + padding + signature + metadata at partition tail
# S91app_mount reads: metadata from partition_end-4096, signature from partition_end-4096-512

set -e
EXT4=""; HASHTREE=""; METADATA=""; SIGNATURE=""; OUTPUT=""; PARTITION_SIZE=""

while [ $# -gt 0 ]; do
    case "$1" in
        --ext4)            EXT4="$2"; shift 2 ;;
        --hashtree)        HASHTREE="$2"; shift 2 ;;
        --metadata)        METADATA="$2"; shift 2 ;;
        --signature)       SIGNATURE="$2"; shift 2 ;;
        --output)          OUTPUT="$2"; shift 2 ;;
        --partition-size)  PARTITION_SIZE="$2"; shift 2 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

[ -z "$EXT4" ] || [ -z "$HASHTREE" ] || [ -z "$METADATA" ] || [ -z "$OUTPUT" ] || [ -z "$PARTITION_SIZE" ] && {
    echo "ERROR: missing params" >&2; exit 1; }
for f in "$EXT4" "$HASHTREE" "$METADATA"; do [ -f "$f" ] || { echo "ERROR: $f not found" >&2; exit 1; }; done

PART_BYTES=$(echo "$PARTITION_SIZE" | awk '/[0-9]+K/ {print int($0)*1024} /[0-9]+M/ {print int($0)*1024*1024} /[0-9]+G/ {print int($0)*1024*1024*1024} /[0-9]+$/ {print int($0)}')

mkdir -p "$(dirname "$OUTPUT")"

E_SZ=$(stat -c%s "$EXT4"); H_SZ=$(stat -c%s "$HASHTREE"); M_SZ=$(stat -c%s "$METADATA")

# Determine signature size (0 if not present)
S_SZ=0
if [ -f "$SIGNATURE" ] && [ -s "$SIGNATURE" ]; then S_SZ=$(stat -c%s "$SIGNATURE"); fi

echo "  ext4:     $E_SZ B   hashtree: $H_SZ B   sig: $S_SZ B   meta: $M_SZ B   part: $PART_BYTES B"

# ext4 + hashtree
cat "$EXT4" "$HASHTREE" > "$OUTPUT"

# Pad to: partition_end - 4096(meta) - sig_size
TAIL_SZ=$((4096 + S_SZ))
PAD=$((PART_BYTES - TAIL_SZ - E_SZ - H_SZ))
[ $PAD -gt 0 ] && dd if=/dev/zero bs=1 count=$PAD >> "$OUTPUT" 2>/dev/null && echo "  padding:   $PAD B"

# Signature right before metadata
if [ $S_SZ -gt 0 ]; then
    cat "$SIGNATURE" >> "$OUTPUT"
    echo "  sign:      $S_SZ B at offset $((PART_BYTES - 4096 - S_SZ))"
fi

# Metadata at last 4KB, padded to 4096
cat "$METADATA" >> "$OUTPUT"
PAD2=$((PART_BYTES - $(stat -c%s "$OUTPUT")))
[ $PAD2 -gt 0 ] && dd if=/dev/zero bs=1 count=$PAD2 >> "$OUTPUT" 2>/dev/null

echo "  output:    $OUTPUT ($(stat -c%s "$OUTPUT") B)"
echo "  meta @     $((PART_BYTES - 4096))"
echo "  sig  @     $((PART_BYTES - 4096 - S_SZ))"
