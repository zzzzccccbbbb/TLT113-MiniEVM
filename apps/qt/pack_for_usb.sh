#!/bin/sh
# Pack Qt runtime + demos for USB stick install (no network needed).
# Usage: ./pack_for_usb.sh [/path/to/output_dir]
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
OUT="${1:-$ROOT/usb_payload}"
TAR=$(cat "$ROOT/runtime/Qt_5.12.5.tar.gz.path")
if [ ! -f "$TAR" ]; then
	echo "ERROR: missing $TAR"
	exit 1
fi

rm -rf "$OUT"
mkdir -p "$OUT/apps_qt" "$OUT"
cp -a "$TAR" "$OUT/Qt_5.12.5.tar.gz"
cp -a "$ROOT/runtime/libmtdev.so.1" "$ROOT/runtime/libpcre2-16.so.0" \
	"$ROOT/runtime/libts.so.0" "$ROOT/runtime/qtenv_mouse.sh" "$OUT/"
cp -a "$ROOT/image_display" "$ROOT/led_control" "$ROOT/scripts" "$OUT/apps_qt/"
mkdir -p "$OUT/apps_qt/runtime"
cp -a "$ROOT/runtime/qtenv_mouse.sh" "$OUT/apps_qt/runtime/"

cat > "$OUT/install_on_board.sh" <<'EOF'
#!/bin/sh
# Run on board after mounting USB here (directory containing this script).
set -e
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
echo "Install from $HERE"
mkdir -p /usr/local /usr/lib /root/apps/qt
cd /usr/local
if [ ! -d Qt_5.12.5 ]; then
	tar xzf "$HERE/Qt_5.12.5.tar.gz"
fi
ln -sfn Qt_5.12.5 Qt-5.12.5
cp -a "$HERE"/libmtdev.so.1 "$HERE"/libpcre2-16.so.0 "$HERE"/libts.so.0 /usr/lib/
cp -a "$HERE/apps_qt/." /root/apps/qt/
chmod +x /root/apps/qt/scripts/*.sh /root/apps/qt/*/bin/* /root/apps/qt/runtime/*.sh
sh /root/apps/qt/scripts/board_check.sh
echo "OK. Run: /root/apps/qt/scripts/board_run_image.sh"
EOF
chmod +x "$OUT/install_on_board.sh" "$OUT/apps_qt/scripts/"*.sh

echo "Packed: $OUT"
echo "Copy this folder to a USB stick, mount on board, then:"
echo "  sh /mnt/usb/install_on_board.sh"
du -sh "$OUT" "$OUT/Qt_5.12.5.tar.gz"
