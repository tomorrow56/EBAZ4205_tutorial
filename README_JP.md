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
│   └── jtag/                      # JTAG関連資料
├── tutorials/                    # チュートリアルプロジェクト
│   ├── 01_blink/                  # LED点滅（基本）
│   ├── 02_blinkspeed/             # LED点滅速度制御
│   ├── 03_pattern/                # パターン表示（HDMI出力）
│   ├── 04_gradation/              # グラデーション表示（HDMI出力）
│   ├── 05_Zynq7000/               # Zynq PS+PL統合プロジェクト
│   ├── 06_hdmi_test/              # HDMI出力テスト
│   ├── ebaz4205_merged.xdc        # 統合制約ファイル（XDC）
│   └── ebaz4205_pin_assign.csv    # ピンアサインCSV
├── cq_pub/                       # CQ出版向け資材
│   ├── 01_led/                    # LEDプロジェクトファイル
│   ├── 02_z7000_ps/               # Zynq7000 PSプロジェクトファイル
│   ├── 03_vitis_classic_z7000_ps/ # Vitis Classic PSプロジェクトファイル
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

- Vitis開発環境の使用
- UART通信
- PS-PL連携

### PS_LCD_test

PS側からのLCD制御テストプロジェクトです。

## 開発環境

- **Vivado**: 2023.2
- **Vitis**: 2023.2

## 参考資料

`documents/`フォルダに以下の資料が含まれています：

- EBAZ4205回路図・説明書・PCBデータ・Linuxイメージ（`documents/EBAZ4205/`）
- Zynq-7000 TRM（テクニカルリファレンスマニュアル）（`documents/xilinx_user_guide/`）
- ブートイメージ（`documents/TF_boot_image/`）
- Ethernet PHY資料（`documents/ether_phy/`）

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
