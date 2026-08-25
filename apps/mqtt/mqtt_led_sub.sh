#!/bin/sh
# MQTT LED bridge using board mosquitto CLI tools.
# Subscribe topic t113/led : payload "on"|"off"|"blink"
#
# Terminal A (board):
#   /root/apps/mqtt/mqtt_led_sub.sh
# Terminal B (board or PC):
#   mosquitto_pub -h <board_ip> -t t113/led -m on

LED="${LED_BIN:-/root/apps/led/led_ctrl}"
BROKER="${MQTT_HOST:-127.0.0.1}"
TOPIC="${MQTT_TOPIC:-t113/led}"

if [ ! -x "$LED" ]; then
	echo "missing $LED — build & deploy led_ctrl first"
	exit 1
fi

echo "subscribe $BROKER $TOPIC"
mosquitto_sub -h "$BROKER" -t "$TOPIC" | while read -r msg; do
	echo "got: $msg"
	case "$msg" in
		on|ON|1) killall led_ctrl 2>/dev/null; "$LED" on ;;
		off|OFF|0) killall led_ctrl 2>/dev/null; "$LED" off ;;
		blink|BLINK) killall led_ctrl 2>/dev/null; "$LED" blink 200 200 & ;;
		*) echo "unknown: $msg (use on|off|blink)" ;;
	esac
done
