# Qt demos（按创龙《2-3-Qt工程编译说明》三步走）

当前烧录的 rootfs **只有** `/etc/qtenv.sh`，**没有** `/usr/local/Qt_5.12.5`，
所以开机停在 bootlogo、没有 Launcher、也没有鼠标箭头。  
要先装上官方 Qt 运行库，再用 Demo 的 `bin` 跑通，最后才用 SDK `qmake` 编源码。

## 目录

| 路径 | 内容 |
|------|------|
| `image_display/` | 官方显示图片 demo（src + 预编译 bin） |
| `led_control/` | 官方 LED UI demo（可用鼠标点） |
| `runtime/` | `qtenv_mouse.sh`、mtdev/ts/pcre 依赖；Qt 大包路径见 `*.path` |
| `scripts/` | 板上检测/运行脚本 |
| `deploy_qt.sh` | PC→板：装 Qt 运行库 + 部署 demo |
| `build_from_sdk.sh` | PC：用 SDK `qmake` 交叉编译 src（需先 `./build.sh qt`） |

## 第 1 步：确认板端 Qt 环境

串口或 ssh 上执行（若还没部署过，会报 NOT READY）：

```sh
sh /root/apps/qt/scripts/board_check.sh
```

离线对照（本机已核对过当前 eMMC rootfs 产物）：

- 有：`/etc/qtenv.sh`，rcS 里有 `Launcher &`
- 无：`/usr/local/Qt_5.12.5`、`Launcher` 二进制  
→ 必须先装 `4-软件资料/Ubuntu/tools/Qt/lib/Qt_5.12.5.tar.gz`

## 第 2 步：用 Demo bin 跑通（优先）

### A. PC 能 ping 通板子时

板子先有 IP（例）：

```sh
ifconfig eth0 10.10.2.50 netmask 255.255.255.0 up
```

PC（WSL）：

```sh
cd ~/TLT113-MiniEVM/apps/qt
chmod +x deploy_qt.sh scripts/*.sh build_from_sdk.sh
./deploy_qt.sh 10.10.2.50
```

板上：

```sh
/root/apps/qt/scripts/board_check.sh
/root/apps/qt/scripts/board_run_image.sh
# 另开会话或 Ctrl+C 后：
/root/apps/qt/scripts/board_run_led.sh
```

期望：HDMI 出 Qt 界面；插 USB 鼠标应能看到指针（`evdevmouse`）。

### B. 暂时没网（串口 + U 盘）

在 PC 上准备 U 盘内容：

```sh
cd ~/TLT113-MiniEVM/apps/qt
./pack_for_usb.sh /mnt/d/qt_usb_payload
```

把 `qt_usb_payload/` 拷到 U 盘，插板子后：

```sh
mkdir -p /mnt/usb
mount /dev/sda1 /mnt/usb   # 节点以实际为准
sh /mnt/usb/install_on_board.sh
/root/apps/qt/scripts/board_run_image.sh
```

## 第 3 步：编 SDK 软浮点 qmake（必做）

官方 `Ubuntu/tools/Qt/lib/Qt_5.12.5.tar.gz` 是**硬浮点**，会报  
`libQt5Widgets.so.5: internal error`。必须用 SDK 编软浮点 Qt。

注意：宿主机 **GCC 11** 会导致官方 `./build.sh qt` **假成功**（只拷了 fonts，没有 qmake）。  
已对 `qendian.h` / `qfloat16.h` 打过 `<limits>` 补丁。请用：

```sh
cd ~/TLT113-MiniEVM/apps/qt
chmod +x rebuild_qt_sf.sh build_from_sdk.sh deploy_qt.sh
./rebuild_qt_sf.sh            # 很久；日志 /tmp/qt_rebuild.log
./build_from_sdk.sh           # 编 demo
./deploy_qt.sh 10.10.2.14     # 部署软浮点库 + bin
```

成功标志：存在

`T113-i_v1.0/platform/framework/qt/.../Qt_5.12.5/bin/qmake`

且 `readelf -A .../libQt5Core.so | grep Tag_ABI_VFP` **无输出**。

## 注意

1. 用 **Demo/qt-demos** 的 soft-float `bin`（`ld-linux.so.3`），不要用  
   `Ubuntu/tools/Qt/bin/image_display`（硬浮点 `armhf`，本板会跑不起来）。
2. Demo 的 RPATH 是 `/usr/local/Qt-5.12.5`（横杠），运行库目录是  
   `Qt_5.12.5`（下划线）；部署脚本会做符号链接。
3. 官方 `/etc/qtenv.sh` 默认 `evdevtouch`；测 USB 鼠标请用  
   `runtime/qtenv_mouse.sh`（含 `evdevmouse`）。
