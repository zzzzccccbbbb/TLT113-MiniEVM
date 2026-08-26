#!/bin/sh
# Run ON the board: Qt LED UI; move USB mouse — cursor should appear.
set -e
ROOT=/root/apps/qt
cd "$ROOT/led_control/bin"
killall Launcher image_display led_control 2>/dev/null || true
command -v fbinit >/dev/null 2>&1 && fbinit || true
# shellcheck disable=SC1091
. "$ROOT/runtime/qtenv_mouse.sh"
# Do NOT add "-plugin evdevmouse" here: qtenv_mouse.sh already sets
# QT_QPA_GENERIC_PLUGINS=evdevmouse:evdevtouch. Loading twice breaks clicks.
echo "starting led_control (Ctrl+C to quit)..."
exec ./led_control -platform linuxfb
