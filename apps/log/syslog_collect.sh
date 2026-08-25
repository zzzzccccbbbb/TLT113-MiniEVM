#!/bin/sh
# Collect basic system health into a daily log file.
# Run on board: /root/apps/log/syslog_collect.sh

LOG_DIR="${LOG_DIR:-/root/logs}"
mkdir -p "$LOG_DIR"
LOG="$LOG_DIR/sys_$(date +%Y%m%d).log"

{
	echo "======== $(date) ========"
	echo "[uptime]"
	uptime
	echo "[mem]"
	free
	echo "[disk]"
	df -h
	echo "[loadavg]"
	cat /proc/loadavg 2>/dev/null
	echo
} >> "$LOG"

echo "appended -> $LOG"
