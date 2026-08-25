#!/bin/sh
#
# Start the preinit
#

# Commented ttyS0
grep "#ttyS0::respawn:" ${TARGET_DIR}/etc/inittab >/dev/null
if [ $? -ne 0 ]; then
    sed -i 's/ttyS0/#&/' ${TARGET_DIR}/etc/inittab
    sed -i 's/ttyAS0/#&/' ${TARGET_DIR}/etc/inittab
fi

# Support autologin with root user
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    sed -i '/console::respawn:-\/sbin\/agetty/d' ${TARGET_DIR}/etc/inittab
    sed -i '/# Put a getty on the serial port/a\console::respawn:-/sbin/agetty --keep-baud 115200,38400,9600 console --autologin root' \
    ${TARGET_DIR}/etc/inittab
fi
