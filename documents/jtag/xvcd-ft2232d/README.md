# xvcd-ft2232d

FT2232D 向け **Xilinx Virtual Cable Daemon (XVC)** 実装。**Linux / Windows 対応。**

[tmbinc/xvcd](https://github.com/tmbinc/xvcd) の TCP/JTAG プロトコル処理を踏襲し、
GPIO の代わりに **FT2232D の MPSSE (Multi-Protocol Synchronous Serial Engine)** を用いて
JTAG を実行します。

---

## 機能

| 機能 | 説明 |
|------|------|
| XVC v1.0 対応 | `getinfo:` / `settck:` / `shift:` コマンドを実装 |
| MPSSE 利用 | bit-bang ではなく MPSSE で高速 JTAG クロック |
| TCK 周波数設定 | `settck:` コマンドまたは `-f` オプションで設定 |
| 最大ベクタ長 | 32768 bits |
| ツール対応 | Vivado Hardware Manager / iMPACT / OpenOCD |
| クロスプラットフォーム | Linux・Windows (MinGW-w64 / MSVC) |

---

## ファイル構成

```
src/
├── mpsse_core.h           # 共通コア: MPSSE/XVC ロジック全体
├── xvcd_ft2232d.c         # Linux 版エントリポイント (POSIX socket + main)
├── xvcd_ft2232d_win.c     # Windows 版エントリポイント (Winsock2 + main)
└── Makefile
```

`mpsse_core.h` に MPSSE 定数・FTDI 操作・XVC コマンドハンドラをすべて集約しており、
Linux 版・Windows 版それぞれのソースからインクルードして使用します。

---

## ハードウェア接続

FT2232D の Channel A (ADBUS) を使用します。

| ADBUS ピン | 信号 | 方向 |
|-----------|------|------|
| ADBUS0 (TCK) | JTAG TCK | 出力 |
| ADBUS1 (TDI) | JTAG TDI | 出力 |
| ADBUS2 (TDO) | JTAG TDO | 入力 |
| ADBUS3 (TMS) | JTAG TMS | 出力 |

> **注意:** Channel B (BDBUS) は使用しません。

---

## 依存ライブラリ

### Linux

| ディストリビューション | コマンド |
|----------------------|---------|
| Ubuntu / Debian | `sudo apt install libftdi1-dev pkg-config` |
| Fedora / RHEL | `sudo dnf install libftdi-devel pkgconfig` |
| macOS (Homebrew) | `brew install libftdi` |

### Windows

[MSYS2](https://www.msys2.org/) (MinGW-w64) を推奨します。

```sh
# MSYS2 MinGW64 シェルで実行
pacman -S mingw-w64-x86_64-libftdi mingw-w64-x86_64-libusb
```

または [libftdi のバイナリリリース](https://www.intra2net.com/en/developer/libftdi/) を
手動で配置することも可能です。

---

## ビルド

### Linux

```sh
cd src
make
```

`pkg-config` で `libftdi1` または `libftdi` を自動検出します。

### Windows (MSYS2 / MinGW-w64)

```sh
cd src
mingw32-make CC=gcc \
             TARGET=xvcd_ft2232d.exe \
             SRC=xvcd_ft2232d_win.c \
             FTDI_CFLAGS="-I/mingw64/include/libftdi1" \
             FTDI_LIBS="-lftdi1 -lusb-1.0 -lws2_32"
```

### Windows (Linux からのクロスコンパイル)

```sh
cd src
make CC=x86_64-w64-mingw32-gcc \
     TARGET=xvcd_ft2232d.exe \
     SRC=xvcd_ft2232d_win.c \
     FTDI_CFLAGS="-I/path/to/libftdi-win/include/libftdi1" \
     FTDI_LIBS="-L/path/to/libftdi-win/lib -lftdi1 -lusb-1.0 -lws2_32"
```

### Windows (MSVC)

```bat
cl /W3 /O2 xvcd_ft2232d_win.c ^
   /I<libftdi-include> ^
   /link /LIBPATH:<libftdi-lib> ftdi1.lib libusb-1.0.lib ws2_32.lib
```

---

## 使用方法

### Linux

```sh
sudo ./xvcd_ft2232d [options]
```

### Windows

```bat
xvcd_ft2232d.exe [options]
```

> Windows では管理者権限は不要ですが、USB ドライバに **WinUSB** または
> **libusb-win32** が必要です。[Zadig](https://zadig.akeo.ie/) でドライバを
> インストールしてください。

### オプション

| オプション | 説明 | デフォルト |
|-----------|------|-----------|
| `-v` | 詳細ログ出力 | なし |
| `-p port` | TCP ポート番号 | 2542 |
| `-V vid` | USB ベンダ ID | 0x0403 |
| `-P pid` | USB プロダクト ID | 0x6010 |
| `-s serial` | USB シリアル番号 (複数デバイス区別用) | なし |
| `-f freq` | TCK 周波数 [Hz] (最大 6000000) | 6000000 |

### 実行例

```sh
# デフォルト設定で起動
sudo ./xvcd_ft2232d -v

# 1 MHz TCK、ポート 3000 で起動
sudo ./xvcd_ft2232d -f 1000000 -p 3000

# 複数デバイスがある場合はシリアル番号で指定
sudo ./xvcd_ft2232d -s FT12345A
```

### Vivado Hardware Manager での接続

1. Vivado を起動し「Open Hardware Manager」
2. 「Open Target → Open New Target」
3. 「JTAG cable type」に「Xilinx Virtual Cable」を選択
4. Host: `localhost`, Port: `2542` (または `-p` で指定したポート)

### OpenOCD での接続

```tcl
# openocd.cfg
interface xvc
xvc_host localhost
xvc_port 2542
```

---

## udev ルール (Linux、sudo なし実行)

```sh
# /etc/udev/rules.d/99-ftdi.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="0403", ATTR{idProduct}=="6010", MODE="0666"
```

```sh
sudo udevadm control --reload-rules && sudo udevadm trigger
```

---

## アーキテクチャ

```
Vivado / iMPACT / OpenOCD
          |
          | TCP (XVC v1.0)
          v
 xvcd_ft2232d (Linux)
 xvcd_ft2232d.exe (Windows)
          |
          | libftdi / USB
          v
 FT2232D MPSSE (Channel A)
          |
          | JTAG (TCK/TDI/TDO/TMS)
          v
 Xilinx FPGA / CPLD
```

### ソースコード構造

```
mpsse_core.h
  ├── MPSSE 定数・ピン定義
  ├── ftdi_read_all(), mpsse_init()
  ├── ftdi_open_device(), ftdi_close_device()
  ├── sread(), swrite()  ← sock_read/sock_write を使用
  └── handle_getinfo(), handle_settck(), handle_shift(), handle_client()

xvcd_ft2232d.c  (Linux)
  ├── sock_t = int, sock_read/sock_write = read/write
  ├── msleep() = nanosleep
  └── main(): SIGPIPE 無視, getopt, TCP サーバループ

xvcd_ft2232d_win.c  (Windows)
  ├── sock_t = SOCKET, sock_read/sock_write = recv/send
  ├── msleep() = Sleep
  ├── getopt() 内蔵実装
  └── main(): SetConsoleCtrlHandler, WSAStartup, TCP サーバループ
```

### XVC プロトコルフロー

```
Client                         Server
  |--- "getinfo:" ------------>|
  |<-- "xvcServer_v1.0:N\n" --|
  |
  |--- "settck:" + u32 ------->|
  |<-- u32 (actual period) ----|
  |
  |--- "shift:" + u32 + TMS[] + TDI[] -->|
  |<------- TDO[] -----------------------|
```

### MPSSE shift 処理

`shift:` コマンドを受信すると、ビット数分の MPSSE TMS コマンド
(`0x6F`) を生成してバッチ送信します。各クロックで TMS・TDI を同時に
出力し、TDO を取り込みます。1 ビットにつき 1 レスポンスバイト (bit7 = TDO)
が返るため、最後にパックして XVC クライアントへ返送します。

---

## ライセンス

MIT License (元の xvcd に準拠)

---

## 参考

- [tmbinc/xvcd](https://github.com/tmbinc/xvcd) - オリジナル GPIO 実装
- [Xilinx XVC プロトコル仕様](https://www.xilinx.com/support/documentation/sw_manuals/xilinx13_3/ug655.pdf)
- [FTDI MPSSE Application Note AN_135](https://ftdichip.com/wp-content/uploads/2020/08/AN_135_MPSSE_Basics.pdf)
- [libftdi](https://www.intra2net.com/en/developer/libftdi/)
- [Zadig (Windows USB ドライバインストーラ)](https://zadig.akeo.ie/)
- [MSYS2](https://www.msys2.org/)
