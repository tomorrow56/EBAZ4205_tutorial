# EBAZ4205 Tutorial
[English](README.md) | 日本語

EBAZ4205（Zynq-7000 SoC搭載ボード）を使用したFPGA/SoC開発チュートリアル集です。

## 概要

このリポジトリは、EBAZ4205ボードを使用してXilinx Zynq-7000 SoCの開発を学ぶためのチュートリアルプロジェクトを含んでいます。

### ターゲットデバイス

- **SoC**: Xilinx Zynq-7000 (xc7z010clg400-1)
- **ボード**: EBAZ4205

## ディレクトリ構成

```
EBAZ4205_tutorial/
├── documents/                    # データシート・回路図・リファレンス資料・ブートイメージ等
│   ├── EBAZ4205/                  # ボード資料（回路図、説明書、イメージ等）
│   ├── xilinx_user_guide/         # Xilinx公式User Guide
│   ├── TF_boot_image/             # ブートイメージ
│   ├── ether_phy/                 # Ethernet PHY関連資料
│   ├── adapter/                   # アダプタ/検証用資材
│   ├── jtag/                      # JTAG関連資料
│   ├── SoC_info.txt               # SoC型番メモ
│   ├── README.md                  # 資料フォルダ概要（英語）
│   └── README_JP.md               # 資料フォルダ概要（日本語）
├── tutorials/                    # チュートリアルプロジェクト
│   ├── 01_blink/                  # LED点滅（基本）
│   ├── 02_blinkspeed/             # LED点滅速度制御
│   ├── 03_pattern/                # パターン表示（HDMI出力）
│   ├── 04_gradation/              # グラデーション表示（HDMI出力）
│   ├── 05_Zynq7000/               # Zynq PS+PL統合プロジェクト
│   ├── 06_hdmi_test/              # AXI4-Lite制御のHDMIパターンジェネレータ
│   ├── 07_lcd_spi/                # PS制御ST7789 SPI LCD（生SPI / LVGL v9）
│   ├── ebaz4205_merged.xdc        # 統合制約ファイル（XDC）
│   └── ebaz4205_pin_assign.csv    # ピンアサインCSV
├── cq_pub/                       # CQ出版向け資材
│   ├── 01_led/                    # LEDプロジェクトファイル
│   ├── 02_z7000_ps/               # Zynq7000 PSプロジェクトファイル
│   ├── 03_vitis_classic_z7000_ps/ # Vitis Classic PSプロジェクトファイル
│   ├── 04_hdmi/                   # HDMIプロジェクトファイル
│   │   ├── PL_HDMI/               # PLベースのHDMI出力
│   │   └── hdmi_axi/              # AXI4-Lite制御のHDMIプロジェクト
│   ├── 05_petalinux/              # PetaLinuxプロジェクトファイル
│   │   ├── project_zynq7000_plnx.xpr.zip
│   │   ├── bsp/
│   │   ├── commands.md
│   │   ├── link.md
│   │   └── files/
│   ├── ebaz4205_merged.xdc        # 統合制約ファイル（XDC）
│   └── ebaz4205_pin_assign.csv    # ピンアサインCSV
└── LICENSE
```

## チュートリアル内容

### 01_blink

RGB LEDの点滅制御を行う基本的なPLプロジェクトです。

- システムクロックの分周
- カウンタによるLED制御
- 制約ファイル（XDC）の設定

### 02_blinkspeed

ボタン入力によるLED点滅速度の制御を学びます。

- デバウンス回路の実装
- ユーザー入力の処理

### 03_pattern

HDMI出力によるパターン表示を行います。

- 同期信号生成（syncgen）
- ピクセルクロック生成（pckgen）
- HDMI出力制御

### 04_gradation

グラデーション表示によるHDMI出力の応用です。

### 05_Zynq7000

Zynq SoCのPS（Processing System）とPL（Programmable Logic）を統合したプロジェクトです。

- GPIO/UART/イーサネット/タイマーなどを含むVivadoブロックデザイン
- LED外部テスト1/2、lwIPエコーサーバー、メモリテスト、ペリフェラルテスト、PS LCDなどのVitisアプリケーション
- UART通信とPS-PL連携

### 06_hdmi_test

PSからのAXI4-Liteレジスタ制御によるHDMIパターンジェネレータです。

- AXI4-Liteスレーブインターフェースによる動的切り替え
- VGA/480p/720pなどのタイミングプリセットと10種類のテストパターン
- ランタイムクロック切替え用MMCM DRP
- TMDS/HDMI出力用Digilent rgb2dvi IP

### 07_lcd_spi

EBAZ4205のPSからST7789 SPI LCD（240×240 px）を駆動するプロジェクトです。

- EMIO GPIOとPS SPI0による制御
- `lcd_spi_ps`：生SPIドライバ（カラーバー/全画面塗りつぶしデモ）
- `lcd_lvgl`：LVGL v9 GUIデモ

## CQ出版向け資材

CQ出版向け記事のプロジェクトファイルやアーカイブを格納しています。

- `01_led/` — LEDプロジェクトファイル
- `02_z7000_ps/` — Zynq7000 PSプロジェクトファイル
- `03_vitis_classic_z7000_ps/` — Vitis Classic PSプロジェクトアーカイブ
- `04_hdmi/` — HDMIプロジェクトファイル
  - `PL_HDMI/` — PLベースのHDMI出力
  - `hdmi_axi/` — AXI4-Lite制御のHDMIプロジェクト
- `05_petalinux/` — PetaLinuxプロジェクトファイル

## 開発環境

- **Vivado**: 2024.2
- **Vitis**: 2024.2

## 参考資料

`documents/`フォルダに以下の資料が含まれています：

- EBAZ4205回路図・説明書・PCBデータ・Linuxイメージ（`documents/EBAZ4205/`）
- Zynq-7000 TRM（テクニカルリファレンスマニュアル）（`documents/xilinx_user_guide/`）
- ブートイメージ（`documents/TF_boot_image/`）
- Ethernet PHY資料（`documents/ether_phy/`）
- SoC型番メモ（`documents/SoC_info.txt`）
- 資料フォルダの概要（`documents/README.md`、`documents/README_JP.md`）

## 記事

- https://qiita.com/tomorrow56/items/7a6340c04b87f584288a

## リソース

- https://github.com/xjtuecho/EBAZ4205/

## 参考購入先

- https://ja.aliexpress.com/item/1005006074065888.html
- SDカードスロット型番: [SHOU HAN MEM2055-00-140-00-A (LCSC C393941)](https://www.lcsc.com/product-detail/C393941.html)

## ライセンス

MIT License

Copyright (c) 2025 tomorrow56 A.K.A. ThousanDIY

## 謝辞

一部のコードは [Cobac.Net](https://www.cobac.net/) のサンプルを基にしています。
