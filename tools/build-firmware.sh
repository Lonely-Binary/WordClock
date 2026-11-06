#!/usr/bin/env bash
#
# Build the distributable firmware images for the web installer.
#
# For each panel, produces a single full-flash image (bootloader + partition
# table + app, starting at offset 0x0) and drops it into
# docs/firmware/<panel>/ next to the manifest that the installer page reads.
#
# Usage:  tools/build-firmware.sh [version] [panel ...]
#         tools/build-firmware.sh 1.1.0                 # every panel
#         tools/build-firmware.sh 1.1.0 weather devops  # just these
#
set -euo pipefail

VERSION="${1:-1.0.0}"
shift || true

# id | sketch | product name | partition scheme
# The Word Clock keeps the default partition table it has always shipped
# with. The other panels carry HTTPS + MQTT and don't fit the default 1.2 MB
# app slot, so they use min_spiffs (1.9 MB app, still with OTA).
PANELS=(
  "wordclock|WordClock/WordClock.ino|Lonely Binary Word Clock|default"
  "weather|WeatherStation/WeatherStation.ino|Lonely Binary Weather Station|min_spiffs"
  "status|StatusBoard/StatusBoard.ino|Lonely Binary Status Board|min_spiffs"
  "couple|CoupleBoard/CoupleBoard.ino|Lonely Binary Couple Message Board|min_spiffs"
  "crypto|CryptoClock/CryptoClock.ino|Lonely Binary Crypto Word Clock|min_spiffs"
  "devops|DevOpsClock/DevOpsClock.ino|Lonely Binary DevOps Word Clock|min_spiffs"
)

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

BUILD_ROOT="$(mktemp -d)"
trap 'rm -rf "$BUILD_ROOT"' EXIT

wanted() {
  [ "$#" -eq 1 ] && return 0          # no panel list given: build everything
  local id="$1"; shift
  for p in "$@"; do [ "$p" = "$id" ] && return 0; done
  return 1
}

for entry in "${PANELS[@]}"; do
  IFS='|' read -r PANEL SKETCH NAME SCHEME <<<"$entry"
  wanted "$PANEL" "$@" || continue

  # CDCOnBoot=cdc routes Serial to the board's native USB port, so buyers (and
  # we) can read the boot log over the same cable they flash with.
  FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=${SCHEME}"
  BUILD_DIR="${BUILD_ROOT}/${PANEL}"
  OUT_DIR="docs/firmware/${PANEL}"

  echo "==> Building ${SKETCH} v${VERSION} (${SCHEME} partitions)"
  arduino-cli compile --fqbn "$FQBN" --libraries libraries --build-path "$BUILD_DIR" "$SKETCH"

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
  # A STABLE filename with no version in it. The version lives in
  # manifest.json, which the installer page reads at runtime.
  rm -f "${OUT_DIR}/${PANEL}-"*.bin
  cp "$MERGED" "${OUT_DIR}/${PANEL}.bin"

  cat > "${OUT_DIR}/manifest.json" <<EOF
{
  "name": "${NAME}",
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
done
