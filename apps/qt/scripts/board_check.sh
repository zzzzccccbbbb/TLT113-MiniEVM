#!/bin/sh
# Run ON the board (serial or ssh). Checks Qt runtime readiness.
set -e
echo "=== hostname ==="
hostname 2>/dev/null || true
echo "=== Qt dir ==="
ls -ld /usr/local/Qt_5.12.5 /usr/local/Qt-5.12.5 2>&1 || true
echo "=== key libs ==="
ls /usr/local/Qt_5.12.5/lib/libQt5Core.so* 2>&1 | head -3 || true
ls /usr/local/Qt_5.12.5/plugins/platforms/libqlinuxfb.so 2>&1 || true
ls /usr/local/Qt_5.12.5/plugins/generic/libqevdevmouseplugin.so 2>&1 || true
echo "=== qtenv ==="
ls -l /etc/qtenv.sh /root/apps/qt/runtime/qtenv_mouse.sh 2>&1 || true
echo "=== fb ==="
ls -l /dev/fb0 2>&1 || true
echo "=== mouse input ==="
grep -A6 'Mouse\|mouse' /proc/bus/input/devices 2>/dev/null || true
echo "=== demo bins ==="
ls -l /root/apps/qt/image_display/bin/image_display \
      /root/apps/qt/led_control/bin/led_control 2>&1 || true
echo "=== Launcher (optional) ==="
command -v Launcher 2>&1 || echo "Launcher not in PATH (ok for demos)"
echo "=== DONE ==="
if [ -d /usr/local/Qt_5.12.5 ] && [ -x /root/apps/qt/image_display/bin/image_display ]; then
	echo "READY: try /root/apps/qt/scripts/board_run_image.sh"
else
	echo "NOT READY: deploy Qt runtime + demos first (PC: apps/qt/deploy_qt.sh)"
	exit 1
fi
