#!/bin/sh
# Run ON the board: show test.jpg fullscreen on HDMI via linuxfb.
set -e
ROOT=/root/apps/qt
cd "$ROOT/image_display/bin"
killall Launcher image_display led_control 2>/dev/null || true
command -v fbinit >/dev/null 2>&1 && fbinit || true
# shellcheck disable=SC1091
. "$ROOT/runtime/qtenv_mouse.sh"
echo "starting image_display (Ctrl+C to quit)..."
exec ./image_display ./test.jpg -platform linuxfb
