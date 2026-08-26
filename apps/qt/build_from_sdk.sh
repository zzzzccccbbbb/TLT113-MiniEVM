#!/bin/sh
# Cross-build Qt demos with SDK soft-float qmake.
# Usage: ./build_from_sdk.sh
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SDK_TOP=$(CDPATH= cd -- "$ROOT/../../T113-i_v1.0" && pwd)

BOARD=tlt113-minievm-emmc
if [ -f "$SDK_TOP/.buildconfig" ]; then
	# shellcheck disable=SC1090
	. "$SDK_TOP/.buildconfig"
	[ -n "$LICHEE_BOARD" ] && BOARD=$LICHEE_BOARD
fi

HOST_BIN="$SDK_TOP/out/t113_i/$BOARD/longan/buildroot/host/usr/bin"
QMAKE="$SDK_TOP/platform/framework/qt/qt-everywhere-src-5.12.5/Qt_5.12.5/bin/qmake"

if [ ! -x "$QMAKE" ]; then
	echo "ERROR: qmake missing: $QMAKE"
	echo "Host GCC11 breaks stock ./build.sh qt (silent fail)."
	echo "Rebuild with:"
	echo "  $ROOT/rebuild_qt_sf.sh"
	exit 1
fi

export PATH="$HOST_BIN:$PATH"
echo "Using qmake: $QMAKE"

for app in image_display led_control; do
	SRC="$ROOT/$app/src"
	OUT="$ROOT/$app/bin"
	mkdir -p "$OUT"
	echo "=== build $app ==="
	(
		cd "$SRC"
		rm -f Makefile .qmake.stash
		"$QMAKE"
		make -j"$(nproc 2>/dev/null || echo 2)"
		if [ -f "$SRC/$app" ]; then
			cp -f "$SRC/$app" "$OUT/$app"
		elif [ -f "$OUT/$app" ]; then
			:
		else
			# DESTDIR / shadow builds
			find "$SRC" -maxdepth 2 -type f -name "$app" -executable -exec cp -f {} "$OUT/$app" \;
		fi
		make clean >/dev/null 2>&1 || true
		rm -f "$SRC/Makefile" "$SRC/.qmake.stash" 2>/dev/null || true
	)
	test -x "$OUT/$app"
	file "$OUT/$app"
	ls -l "$OUT/$app"
done

echo "Built OK. Deploy soft-float Qt + bins:"
echo "  $ROOT/deploy_qt.sh <board_ip>"
