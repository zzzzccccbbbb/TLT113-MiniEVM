# T113 soft-float toolchain (matches .buildconfig gnueabi)
CROSS ?= arm-linux-gnueabi-
CC    := $(CROSS)gcc
CFLAGS ?= -O2 -Wall -Wextra

SDK_TOP ?= $(abspath ../T113-i_v1.0)
TOOLCHAIN_BIN ?= $(SDK_TOP)/out/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin

export PATH := $(TOOLCHAIN_BIN):$(PATH)

TARGET = led_ctrl
BOARD_IP ?= 10.10.2.50

.PHONY: all clean deploy

all: $(TARGET)

$(TARGET): led_ctrl.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

deploy: $(TARGET)
	scp $(TARGET) root@$(BOARD_IP):/root/apps/led/
	@echo "On board: chmod +x /root/apps/led/$(TARGET) && /root/apps/led/$(TARGET) blink"
