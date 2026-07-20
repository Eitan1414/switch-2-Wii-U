#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

if [[ ! -f "$ROOT/app/content/intro/frames/frame_0181.webp" ]]; then
  "$ROOT/scripts/prepare_intro.sh"
fi

/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake \
  -S "$ROOT/app" -B "$ROOT/build/app" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build/app" --parallel 2

make -C "$ROOT/plugin" clean
make -C "$ROOT/plugin" -j2

rm -rf "$ROOT/dist"
mkdir -p \
  "$ROOT/dist/sd/wiiu/apps" \
  "$ROOT/dist/sd/wiiu/environments/aroma/plugins"

cp "$ROOT/build/app/Switch2Mode.wuhb" \
  "$ROOT/dist/sd/wiiu/apps/Switch2Mode.wuhb"
cp "$ROOT/plugin/Switch2ModePlugin.wps" \
  "$ROOT/dist/sd/wiiu/environments/aroma/plugins/Switch2ModePlugin.wps"

cd "$ROOT/dist/sd"
zip -r "$ROOT/dist/Switch2Mode_SD.zip" wiiu

echo "Artifacts:"
echo "  $ROOT/dist/Switch2Mode_SD.zip"
echo "  $ROOT/build/app/Switch2Mode.wuhb"
echo "  $ROOT/plugin/Switch2ModePlugin.wps"
