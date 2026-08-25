#!/bin/sh
# Publish a test MQTT message.
# Usage: mqtt_pub_test.sh [on|off|blink] [broker]

MSG="${1:-on}"
BROKER="${2:-127.0.0.1}"
TOPIC="${MQTT_TOPIC:-t113/led}"

mosquitto_pub -h "$BROKER" -t "$TOPIC" -m "$MSG"
echo "published $TOPIC = $MSG -> $BROKER"
