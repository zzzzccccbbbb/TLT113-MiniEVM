#!/bin/sh
# Build all C demos with SDK soft-float toolchain.
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SDK_TOP=$(CDPATH= cd -- "$ROOT/../T113-i_v1.0" && pwd)
TOOLCHAIN_BIN="$SDK_TOP/out/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin"
export PATH="$TOOLCHAIN_BIN:$PATH"

echo "Using: $(command -v arm-linux-gnueabi-gcc)"
make -C "$ROOT/led" clean all
make -C "$ROOT/net" clean all
echo "Built:"
ls -l "$ROOT/led/led_ctrl" "$ROOT/net/tcp_echo"
