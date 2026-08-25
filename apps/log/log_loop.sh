#!/bin/sh
# Simple loop collector when crond is unavailable.
# Usage: /root/apps/log/log_loop.sh [interval_sec]

INTERVAL="${1:-60}"
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

while true; do
	"$SCRIPT_DIR/syslog_collect.sh"
	sleep "$INTERVAL"
done
