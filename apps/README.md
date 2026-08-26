# T113 MiniEVM 用户态入门示例

不改内核、不重编固件。在 PC（WSL）交叉编译后上传到板子运行。

## 目录

| 路径 | 内容 |
|------|------|
| `led/` | `led_ctrl`：sysfs 控 `user-led0`（on/off/blink/breath） |
| `log/` | 系统负载采集脚本、可选开机助手 |
| `mqtt/` | 用板载 mosquitto 收消息控灯 |
| `net/` | 最小 TCP echo 服务 |
| `qt/` | Qt demo（先装运行库 + 官方 bin，再 SDK qmake）；见 `qt/README.md` |

LED 为 GPIO（PG11），breath 为软件占空比，不是硬件 PWM。

## 1. 准备网络

板子要有 IP（上次若 DHCP 失败可临时静态）：

```sh
ifconfig eth0 10.10.2.50 netmask 255.255.255.0 up
```

PC 能 `ping` / `ssh root@板IP`。

## 2. 一键编译 + 部署

```sh
cd apps
chmod +x build_all.sh deploy.sh log/*.sh mqtt/*.sh
./deploy.sh 10.10.2.50    # 改成你的板子 IP
```

或只编译：

```sh
./build_all.sh
```

## 3. 板上验证

```sh
# LED
/root/apps/led/led_ctrl on
/root/apps/led/led_ctrl off
/root/apps/led/led_ctrl blink 200 200
# Ctrl+C 结束；呼吸灯：
/root/apps/led/led_ctrl breath 800

# 日志
/root/apps/log/syslog_collect.sh
cat /root/logs/sys_*.log

# MQTT 控灯（两个 ssh 会话）
# A:
/root/apps/mqtt/mqtt_led_sub.sh
# B:
/root/apps/mqtt/mqtt_pub_test.sh on
/root/apps/mqtt/mqtt_pub_test.sh off
/root/apps/mqtt/mqtt_pub_test.sh blink

# TCP echo
/root/apps/net/tcp_echo 5000
# PC: nc <板IP> 5000
```

## 4. 可选开机自启

板上备份后，在 `/etc/init.d/rcS` 末尾加一行：

```sh
/root/apps/log/S99hello_demo start
```

或登录后手动：

```sh
/root/apps/log/S99hello_demo start
```

## 工具链

使用 SDK 软浮点：`arm-linux-gnueabi-gcc`  
路径：`T113-i_v1.0/out/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin`

不要用 `gnueabihf`。

## 串口说明

当前板级 DTS 仅启用 `uart0`（调试口）。其它 UART 需改 DTS 并重编内核，不属于本目录「纯应用」范围。
