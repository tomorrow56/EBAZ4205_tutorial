# CQ出版向け資材

[English](README.md) | 日本語

このフォルダには、CQ出版向けのEBAZ4205関連記事で使用したプロジェクトファイルやアーカイブを格納しています。

## 概要

このディレクトリには、各記事連載に対応するVivado / Vitisプロジェクト
アーカイブ、ソースファイル、エクスポートされたハードウェアファイルを
格納しています。番号付きのサブフォルダは、記事シリーズの各プロジェクト
や段階に対応しています。

## ディレクトリ構成

```
cq_pub/
├── 01_led/                         # LED点滅プロジェクト
│   ├── led_project_01.xpr.zip      # Vivadoプロジェクトアーカイブ
│   ├── pkg/                        # パッケージング用スクリプトと制約
│   │   ├── bd.tcl
│   │   ├── constrs_1.xdc
│   │   └── setup.tcl
│   └── sim/                        # シミュレーション用ファイル
│       ├── tb_led_blink.sv
│       └── tb_led_blink_behav.wcfg
├── 02_z7000_ps/                    # Zynq7000 PSプロジェクト
│   ├── project_zynq7000_all.xpr.zip # Vivadoプロジェクトアーカイブ
│   └── files/                      # エクスポートされたハードウェアファイル
│       ├── EBAZ4205.xdc
│       ├── Zynq7000_all_wrapper.xsa
│       └── ps_setting.tcl
├── 03_vitis_classic_z7000_ps/      # Vitis Classic PSアプリケーション
│   ├── vitis_EBAZ4205_ps_test_export_archive.ide.zip
│   └── files/
│       ├── Zynq7000_all_wrapper.xsa
│       ├── lcd_ext_ps/
│       │   └── main.c
│       └── led_ext/
│           └── main.c
├── 04_hdmi/                        # HDMI出力プロジェクト
│   ├── PL_HDMI/                    # PLのみのHDMIカラーバー
│   │   ├── vivado_PL_HDMI.xpr.zip
│   │   └── files/
│   │       ├── PL_HDMI.xdc
│   │       ├── PL_HDMI_wrapper.v
│   │       ├── color_bar.v
│   │       └── rgb2dvi/
│   └── hdmi_axi/                   # AXI4-Lite制御のHDMIパターンジェネレータ
│       ├── vivado_hdmi_axi.xpr.zip
│       ├── vivado_hdmi_axi/
│       │   └── files/
│       │       ├── hdmi_axi_720p.xdc
│       │       ├── hdmi_axi_ps.tcl
│       │       ├── hdmi_axi_src_720p/
│       │       └── system_wrapper.xsa
│       └── vitis_hdmi_axi/
│           ├── vitis_hdmi_axi_archive.ide.zip
│           └── files/
│               ├── hdmi_axi.c
│               ├── hdmi_axi.h
│               └── main.c
├── ebaz4205_merged.xdc             # 統合制約ファイル
└── ebaz4205_pin_assign.csv         # ピンアサインCSV
```

## 内容

### 01_led — LED点滅プロジェクト

最初の記事用の、PLのみのシンプルなLED点滅プロジェクトです。

- `led_project_01.xpr.zip` — Vivadoプロジェクトアーカイブ
- `pkg/` — パッケージング用スクリプトと制約
  - `bd.tcl` — ブロックデザインTclスクリプト
  - `constrs_1.xdc` — 制約ファイル
  - `setup.tcl` — プロジェクトセットアップスクリプト
- `sim/` — シミュレーションテストベンチ
  - `tb_led_blink.sv` — Verilogテストベンチ
  - `tb_led_blink_behav.wcfg` — 動作シミュレーション用波形設定

### 02_z7000_ps — Zynq7000 PSプロジェクト

EBAZ4205向けのZynq PSハードウェアプロジェクトです。

- `project_zynq7000_all.xpr.zip` — Vivadoプロジェクトアーカイブ
- `files/`
  - `EBAZ4205.xdc` — ボード用ピン制約
  - `Zynq7000_all_wrapper.xsa` — エクスポートされたハードウェアハンドオフファイル
  - `ps_setting.tcl` — Zynq PS設定Tclスクリプト

### 03_vitis_classic_z7000_ps — Vitis Classic PSアプリケーション

Zynq PSアプリケーション向けのVitis Classicワークスペースアーカイブとサンプルソースファイルです。

- `vitis_EBAZ4205_ps_test_export_archive.ide.zip` — Vitisワークスペースアーカイブ
- `files/`
  - `Zynq7000_all_wrapper.xsa` — ハードウェアエクスポート
  - `lcd_ext_ps/main.c` — LCD外部制御サンプル
  - `led_ext/main.c` — LED外部制御サンプル

### 04_hdmi — HDMI出力プロジェクト

EBAZ4205向けのHDMI出力プロジェクトです。

#### PL_HDMI

PLのみのHDMIカラーバー出力プロジェクトです。

- `vivado_PL_HDMI.xpr.zip` — Vivadoプロジェクトアーカイブ
- `files/`
  - `PL_HDMI.xdc` — ピン/クロック制約
  - `PL_HDMI_wrapper.v` — トップレベルラッパー
  - `color_bar.v` — カラーバー生成器
  - `rgb2dvi/` — Digilent rgb2dvi IPソース

#### hdmi_axi

PS連携によるAXI4-Lite制御のHDMIパターンジェネレータです。

- `vivado_hdmi_axi.xpr.zip` — Vivadoプロジェクトアーカイブ
- `vivado_hdmi_axi/files/`
  - `hdmi_axi_720p.xdc` — 720p制約
  - `hdmi_axi_ps.tcl` — PS設定スクリプト
  - `hdmi_axi_src_720p/` — RTLソース
  - `system_wrapper.xsa` — ハードウェアエクスポート
- `vitis_hdmi_axi/`
  - `vitis_hdmi_axi_archive.ide.zip` — Vitisワークスペースアーカイブ
  - `files/`
    - `hdmi_axi.c` / `hdmi_axi.h` — AXIドライバ
    - `main.c` — アプリケーションエントリ

## 共通ファイル

- `ebaz4205_merged.xdc` — EBAZ4205用統合制約ファイル
- `ebaz4205_pin_assign.csv` — ピンアサインCSV

## 注意

`.xpr.zip` および `.ide.zip` は、Vivado / Vitis で
エクスポートされたプロジェクトです。インポートには対応する
Xilinxツールを使用してください。
