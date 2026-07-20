#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
INPUT="${1:-$ROOT/Boot_WiiU_Switch2_16x9_720p_Final.mp4}"
FRAMES="$ROOT/app/content/intro/frames"
META="$ROOT/app/meta"

if [[ ! -f "$INPUT" ]]; then
  echo "Video introuvable: $INPUT" >&2
  exit 1
fi

rm -rf "$FRAMES"
mkdir -p "$FRAMES" "$META"

ffmpeg -v error -y -i "$INPUT" \
  -vf "fps=20,scale=960:540:flags=lanczos" \
  -c:v libwebp -quality 72 -compression_level 6 -an \
  "$FRAMES/frame_%04d.webp"

ffmpeg -v error -y -i "$INPUT" -vn -c:a libvorbis -q:a 4 \
  "$ROOT/app/content/intro/intro.ogg"

ffmpeg -v error -y -ss 0 -i "$INPUT" -frames:v 1 \
  -vf "scale=1280:720:flags=lanczos" "$META/tv-splash.png"
ffmpeg -v error -y -ss 0 -i "$INPUT" -frames:v 1 \
  -vf "scale=854:480:flags=lanczos" "$META/drc-splash.png"
ffmpeg -v error -y -ss 8 -i "$INPUT" -frames:v 1 \
  -vf "crop=720:720:280:0,scale=128:128:flags=lanczos" "$META/icon.png"

count=$(find "$FRAMES" -name 'frame_*.webp' | wc -l)
if [[ "$count" -ne 181 ]]; then
  echo "Attention: 181 images attendues, $count creees." >&2
fi

echo "$count images et la piste audio ont ete preparees."

