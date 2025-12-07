# EBAZ4205 Tutorial

EBAZ4205（Zynq-7000 SoC搭載ボード）を使用したFPGA/SoC開発チュートリアル集です。

## 概要

このリポジトリは、EBAZ4205ボードを使用してXilinx Zynq-7000 SoCの開発を学ぶためのチュートリアルプロジェクトを含んでいます。

### ターゲットデバイス

- **SoC**: Xilinx Zynq-7000 (xc7z010clg400-1)
- **ボード**: EBAZ4205

## ディレクトリ構成

```
EBZA4205_tutorial/
├── EBAZ4205_data/          # データシート・回路図・リファレンス資料
│   ├── EBAZ4205-Instruction.pdf
│   ├── EBAZ4205-SCH.pdf
│   ├── EBAZ4205_schematic.pdf
│   ├── ug585-Zynq-7000-TRM.pdf
│   └── ...
├── tutorials/              # チュートリアルプロジェクト
│   ├── 01_blink/          # LED点滅（基本）
│   ├── 02_blinkspeed/     # LED点滅速度制御
│   ├── 03_pattern/        # パターン表示（HDMI出力）
│   ├── 04_gradation/      # グラデーション表示（HDMI出力）
│   ├── 05_Zynq7000/       # Zynq PS+PL統合プロジェクト
│   └── PS_LCD_test/       # PS側LCDテスト
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

`EBAZ4205_data/`フォルダに以下の資料が含まれています：

- EBAZ4205回路図
- Zynq-7000 TRM（テクニカルリファレンスマニュアル）
- ボード説明書
- Linuxブートイメージ

## ライセンス

MIT License

Copyright (c) 2025 tomorrow56 A.K.A. ThousanDIY

## 謝辞

一部のコードは [Cobac.Net](https://www.cobac.net/) のサンプルを基にしています。
