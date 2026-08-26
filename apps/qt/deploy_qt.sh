#!/bin/sh
# Deploy soft-float Qt runtime (from SDK build) + demo bins to board.
# Usage: ./deploy_qt.sh [board_ip]
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
IP="${1:-10.10.2.50}"
SDK_TOP=$(CDPATH= cd -- "$ROOT/../../T113-i_v1.0" && pwd)
SDK_QT="$SDK_TOP/platform/framework/qt/qt-everywhere-src-5.12.5/Qt_5.12.5"
STAGE="$ROOT/runtime/sf_stage"
BOARD=tlt113-minievm-emmc
if [ -f "$SDK_TOP/.buildconfig" ]; then
	# shellcheck disable=SC1090
	. "$SDK_TOP/.buildconfig"
	[ -n "$LICHEE_BOARD" ] && BOARD=$LICHEE_BOARD
fi
TARGET_LIB="$SDK_TOP/out/t113_i/$BOARD/longan/buildroot/target/usr/lib"

echo "Board: root@$IP"
ping -c 1 -W 2 "$IP" >/dev/null

ssh -o ConnectTimeout=5 "root@$IP" \
	"mkdir -p /root/apps/qt/runtime /usr/local /usr/lib /tmp/qt_deploy"

if [ -d "$SDK_QT/lib" ] && ls "$SDK_QT/lib"/libQt5Core.so* >/dev/null 2>&1; then
	echo "Packing soft-float Qt from SDK: $SDK_QT"
	rm -rf "$STAGE"
	mkdir -p "$STAGE/Qt_5.12.5"
	cp -a "$SDK_QT/lib" "$SDK_QT/plugins" "$STAGE/Qt_5.12.5/" 2>/dev/null || true
	[ -d "$SDK_QT/qml" ] && cp -a "$SDK_QT/qml" "$STAGE/Qt_5.12.5/" || true
	[ -d "$SDK_QT/fonts" ] && cp -a "$SDK_QT/fonts" "$STAGE/Qt_5.12.5/" || \
		cp -a "$SDK_TOP/platform/framework/qt/qt-everywhere-src-5.12.5/fonts" "$STAGE/Qt_5.12.5/" 2>/dev/null || true
	rm -rf "$STAGE/Qt_5.12.5/lib/cmake" "$STAGE/Qt_5.12.5/lib/pkgconfig" \
		"$STAGE/Qt_5.12.5/lib"/*.a "$STAGE/Qt_5.12.5/lib"/*.prl "$STAGE/Qt_5.12.5/lib"/*.la 2>/dev/null || true
	tar -C "$STAGE" -czf "$ROOT/runtime/Qt_5.12.5_sf.tar.gz" Qt_5.12.5
	QT_TAR="$ROOT/runtime/Qt_5.12.5_sf.tar.gz"
	echo "ABI check (must NOT show Tag_ABI_VFP_args):"
	readelf -A "$STAGE/Qt_5.12.5/lib/libQt5Core.so"* 2>/dev/null | grep -E 'Tag_ABI_VFP|Tag_FP' | head -5 || true
else
	echo "ERROR: SDK soft-float Qt missing. Run: $ROOT/rebuild_qt_sf.sh"
	exit 1
fi

# Soft-float deps from buildroot target — NEVER use Ubuntu/tools hard-float .so
mkdir -p "$ROOT/runtime/sf_deps"
for name in libpcre2-16.so.0 libmtdev.so.1 libts.so.0; do
	src=$(readlink -f "$TARGET_LIB/$name" 2>/dev/null || true)
	if [ -z "$src" ] || [ ! -f "$src" ]; then
		echo "ERROR: missing soft-float $TARGET_LIB/$name"
		exit 1
	fi
	cp -aL "$TARGET_LIB/$name" "$ROOT/runtime/sf_deps/$name"
done
echo "Deps ABI:"
readelf -A "$ROOT/runtime/sf_deps/libpcre2-16.so.0" | grep Tag_ABI_VFP || echo "  libpcre2-16: soft-float OK"

echo "Upload Qt runtime + soft-float deps..."
scp "$QT_TAR" "root@$IP:/tmp/qt_deploy/Qt_5.12.5.tar.gz"
scp "$ROOT/runtime/sf_deps/"* \
	"$ROOT/runtime/qtenv_mouse.sh" \
	"root@$IP:/tmp/qt_deploy/"

echo "Upload demos + scripts..."
scp -r "$ROOT/image_display" "$ROOT/led_control" "$ROOT/scripts" \
	"root@$IP:/root/apps/qt/"
scp "$ROOT/runtime/qtenv_mouse.sh" "root@$IP:/root/apps/qt/runtime/"

ssh "root@$IP" 'set -e
cp -a /tmp/qt_deploy/qtenv_mouse.sh /root/apps/qt/runtime/
chmod +x /root/apps/qt/scripts/*.sh /root/apps/qt/*/bin/* /root/apps/qt/runtime/*.sh
rm -rf /usr/local/Qt_5.12.5
cd /usr/local
tar xzf /tmp/qt_deploy/Qt_5.12.5.tar.gz
ln -sfn Qt_5.12.5 Qt-5.12.5
# Restore soft-float deps (overwrite any previous hard-float copies)
cp -a /tmp/qt_deploy/libpcre2-16.so.0 /tmp/qt_deploy/libmtdev.so.1 /tmp/qt_deploy/libts.so.0 /usr/lib/
ldconfig 2>/dev/null || true
ls -ld /usr/local/Qt_5.12.5 /usr/local/Qt-5.12.5
ls -l /usr/lib/libpcre2-16.so.0
sh /root/apps/qt/scripts/board_check.sh
'

echo ""
echo "Deployed soft-float Qt. On board ($IP):"
echo "  /root/apps/qt/scripts/board_run_image.sh"
echo "  /root/apps/qt/scripts/board_run_led.sh"
