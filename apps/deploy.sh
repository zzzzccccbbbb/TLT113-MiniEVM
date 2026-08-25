#!/bin/sh
# Deploy demos to board. Usage: ./deploy.sh [board_ip]
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
IP="${1:-10.10.2.50}"

"$ROOT/build_all.sh"

ssh "root@$IP" "mkdir -p /root/apps/led /root/apps/log /root/apps/mqtt /root/apps/net"

scp "$ROOT/led/led_ctrl" "root@$IP:/root/apps/led/"
scp "$ROOT/net/tcp_echo" "root@$IP:/root/apps/net/"
scp "$ROOT/log/"*.sh "$ROOT/log/S99hello_demo" "root@$IP:/root/apps/log/"
scp "$ROOT/mqtt/"*.sh "root@$IP:/root/apps/mqtt/"

ssh "root@$IP" "chmod +x /root/apps/led/led_ctrl /root/apps/net/tcp_echo /root/apps/log/* /root/apps/mqtt/*"

echo "Deployed to root@$IP:/root/apps/"
echo "Try on board:"
echo "  /root/apps/led/led_ctrl blink"
echo "  /root/apps/log/syslog_collect.sh"
echo "  /root/apps/mqtt/mqtt_led_sub.sh"
echo "  /root/apps/net/tcp_echo 5000"
