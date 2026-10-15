#!/bin/bash
# rauc.sh — Wrapper for RAUC 1.15.2 host tool
# Bundled with required shared libraries for Ubuntu 18.04 / d9-sdk-build Docker
#
# Usage: ./tools/rauc.sh <rauc args>
#   e.g. ./tools/rauc.sh bundle --cert=... --key=... input/ output.raucb
#
# This wrapper sets LD_LIBRARY_PATH to use the bundled shared libs,
# so rauc works without installing system-wide dependencies.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RAUC_BIN="${SCRIPT_DIR}/bin/rauc"
RAUC_LIB="${SCRIPT_DIR}/lib"

if [ ! -x "$RAUC_BIN" ]; then
    echo "ERROR: rauc binary not found at ${RAUC_BIN}" >&2
    echo "Run ./tools/install-rauc.sh to build and install." >&2
    exit 1
fi

export LD_LIBRARY_PATH="${RAUC_LIB}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$RAUC_BIN" "$@"
