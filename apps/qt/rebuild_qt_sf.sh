#!/bin/sh
# Rebuild soft-float Qt for T113 MiniEVM with error checks.
# Fixes: previous ./build.sh qt reported OK but host qmake failed on GCC11.
set -e
SDK_TOP=$(CDPATH= cd -- "$(dirname -- "$0")/../../T113-i_v1.0" && pwd)
LOG=${1:-/tmp/qt_rebuild.log}

cd "$SDK_TOP"
set -a
# shellcheck disable=SC1091
. ./.buildconfig
set +a

export AW_QT_VER=5.12.5
export LICHEE_QT_DIR=${LICHEE_PLATFORM_DIR}/framework/qt/qt-everywhere-src-${AW_QT_VER}
export QT_INSTALL_DIR=$LICHEE_QT_DIR/Qt_${AW_QT_VER}
export QT_RUN_DIR=/usr/local/Qt_${AW_QT_VER}
export QT_TARGET_DIR=$LICHEE_BR_OUT/target/${QT_RUN_DIR}

echo "=== rebuild Qt soft-float ===" | tee "$LOG"
echo "QT_INSTALL_DIR=$QT_INSTALL_DIR" | tee -a "$LOG"
echo "QT_TARGET_DIR=$QT_TARGET_DIR" | tee -a "$LOG"
date | tee -a "$LOG"

cd "$LICHEE_QT_DIR"
# shellcheck disable=SC1091
. ./buildsetup_sf.sh

echo "=== qtmakeconfig ===" | tee -a "$LOG"
qtmakeconfig >>"$LOG" 2>&1
test -f "$LICHEE_QT_DIR/Makefile" || {
	echo "ERROR: configure failed (no Makefile). See $LOG" | tee -a "$LOG"
	exit 1
}

echo "=== qtmakeall (long) ===" | tee -a "$LOG"
qtmakeall >>"$LOG" 2>&1

echo "=== qtmakeinstall ===" | tee -a "$LOG"
qtmakeinstall >>"$LOG" 2>&1

QMAKE="$QT_INSTALL_DIR/bin/qmake"
CORELIB=$(ls "$QT_INSTALL_DIR/lib"/libQt5Core.so* 2>/dev/null | head -1 || true)
if [ ! -x "$QMAKE" ] || [ -z "$CORELIB" ]; then
	echo "ERROR: install incomplete: qmake=$QMAKE core=$CORELIB" | tee -a "$LOG"
	exit 1
fi

echo "=== ABI check (should NOT have Tag_ABI_VFP_args) ===" | tee -a "$LOG"
readelf -A "$CORELIB" | grep -E 'VFP|Tag_ABI' | tee -a "$LOG" || true

echo "OK: $QMAKE" | tee -a "$LOG"
ls -l "$QMAKE" "$CORELIB" | tee -a "$LOG"
date | tee -a "$LOG"
