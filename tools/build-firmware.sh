#!/usr/bin/env bash
#
# Build the distributable firmware image for the web installer.
#
# Produces a single full-flash image (bootloader + partition table + app,
# starting at offset 0x0) and drops it into docs/firmware/<panel>/ next to the
# manifest that the installer page reads.
#
# Usage:  tools/build-firmware.sh [version]
#         tools/build-firmware.sh 1.0.1
#
set -euo pipefail

VERSION="${1:-1.0.0}"
PANEL="wordclock"
SKETCH="WordClock/WordClock.ino"
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

BUILD_DIR="$(mktemp -d)"
OUT_DIR="docs/firmware/${PANEL}"
trap 'rm -rf "$BUILD_DIR"' EXIT

echo "==> Building ${SKETCH} v${VERSION}"
# CDCOnBoot=cdc routes Serial to the board's native USB port, so buyers (and we)
# can read the boot log over the same cable they flash with. Without it Serial
# goes to the UART pins and the USB port stays silent.
arduino-cli compile --fqbn "$FQBN" --build-path "$BUILD_DIR" "$SKETCH"

MERGED="${BUILD_DIR}/$(basename "$SKETCH").merged.bin"
if [ ! -f "$MERGED" ]; then
  echo "!! arduino-cli did not produce a merged image at ${MERGED}" >&2
  exit 1
fi

# Sanity check: a valid ESP image starts with magic byte 0xE9, both at the
# bootloader (0x0) and at the app offset (0x10000).
python3 - "$MERGED" <<'PY'
import sys
d = open(sys.argv[1], 'rb').read()
assert d[0] == 0xE9,        f"bad bootloader magic: {hex(d[0])}"
assert d[0x10000] == 0xE9,  f"bad app magic: {hex(d[0x10000])}"
print(f"    image OK: {len(d)} bytes, magic verified at 0x0 and 0x10000")
PY

mkdir -p "$OUT_DIR"
# Deliberately a STABLE filename with no version in it. The version lives in
# manifest.json, and the installer page reads it from there at runtime — so
# bumping a version never leaves a stale hardcoded link in docs/index.html.
rm -f "${OUT_DIR}/${PANEL}-"*.bin      # clean up the old versioned scheme
cp "$MERGED" "${OUT_DIR}/${PANEL}.bin"

cat > "${OUT_DIR}/manifest.json" <<EOF
{
  "name": "Lonely Binary Word Clock",
  "version": "${VERSION}",
  "new_install_prompt_erase": false,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "${PANEL}.bin", "offset": 0 }
      ]
    }
  ]
}
EOF

echo "==> Wrote ${OUT_DIR}/${PANEL}.bin + manifest.json (v${VERSION})"
