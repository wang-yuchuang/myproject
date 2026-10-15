#!/bin/bash
# install-rauc.sh — Build RAUC 1.15.2 from Buildroot source in Docker
#
# Prerequisites: d9-sdk-build:2.11 Docker image must be available
#
# This script compiles rauc 1.15.2 with minimal features (no service/network)
# and bundles it with all required shared libraries into tools/bin + tools/lib/
#
# Usage: ./tools/install-rauc.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SDK_TOP="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DOCKER_IMAGE="d9-sdk-build:2.11"

echo "[install-rauc] Building RAUC 1.15.2 in Docker ${DOCKER_IMAGE}..."

# Find Buildroot rauc source directory
RAUC_SRC=$(find "${SDK_TOP}/out/" -path "*/build/rauc-1.15.2/meson.build" -type f 2>/dev/null | head -1)
if [ -z "$RAUC_SRC" ]; then
    echo "ERROR: RAUC 1.15.2 source not found in Buildroot output." >&2
    echo "Run './build.sh buildroot docker' first to populate the build tree." >&2
    exit 1
fi
RAUC_SRC_DIR="$(dirname "$RAUC_SRC")"
echo "[install-rauc] Source: ${RAUC_SRC_DIR}"

# Build in Docker
docker run --rm \
    -v "${SDK_TOP}:/sdk" \
    -v "${SCRIPT_DIR}:/tools" \
    "${DOCKER_IMAGE}" bash -c "
set -e
apt-get update -qq 2>/dev/null
apt-get install -y -qq libjson-glib-dev 2>&1 | tail -1

RAUC_SRC='${RAUC_SRC_DIR}'
cd \"\$RAUC_SRC\"
rm -rf _host_build

echo '[install-rauc] Configuring...'
meson setup _host_build --prefix=/tools \
    --wrap-mode=nofallback \
    -Dservice=false \
    -Dnetwork=false \
    -Dstreaming=false \
    -Dtests=false \
    -Dgpt=disabled \
    -Dcomposefs=disabled \
    -Dpkcs11_engine=false \
    2>&1 | tail -5

echo '[install-rauc] Compiling...'
ninja -C _host_build -j\$(nproc) 2>&1 | tail -5

echo '[install-rauc] Installing binary...'
mkdir -p /tools/bin /tools/lib
cp _host_build/rauc /tools/bin/rauc
chmod +x /tools/bin/rauc

echo '[install-rauc] Bundling shared libraries...'
for lib in \$(ldd /tools/bin/rauc | grep -o '/[^ ]*'); do
    cp -L \"\$lib\" /tools/lib/ 2>/dev/null || true
done
cp -L /lib64/ld-linux-x86-64.so.2 /tools/lib/ 2>/dev/null || true

echo '[install-rauc] Verifying...'
/tools/bin/rauc --version
echo 'Done!'
"

echo "[install-rauc] Installed to ${SCRIPT_DIR}/bin/rauc"
echo "[install-rauc] Use: ${SCRIPT_DIR}/rauc.sh <args>"
