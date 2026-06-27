# LCD_SPI — EBAZ4205 PS SPI LCDドライバプロジェクト

[English](README.md) | 日本語

このフォルダには、EBAZ4205ボードのZynq-7000 PS（Processing System）から
**ST7789** SPI LCD（240×240 px）を制御する2つのVitisアプリケーションプロジェクトが含まれています。

## ハードウェア構成

| 信号   | EMIOピン | 説明                              |
|--------|----------|-----------------------------------|
| CS     | EMIO 54  | チップセレクト（GPIO手動制御）    |
| CD     | EMIO 55  | コマンド / データ切り替え         |
| RES    | EMIO 56  | ハードウェアリセット              |
| SCK / MOSI | PS SPI0 | PS SPIハードウェアで制御       |

- **SPIモード**: Mode 3 (CPOL=1, CPHA=1)
- **クロック分周比**: CPU\_1x / 4
- **ピクセルフォーマット**: RGB565（16 bit/pixel、ビッグエンディアン）
- **解像度**: 240 × 240 ピクセル
- **UART ボーレート**: 115200 bps（ps7\_uart）

## ディレクトリ構成

```text
LCD_SPI/
├── lcd_spi_ps/          # プロジェクト1: 生SPIドライバ（GUIライブラリなし）
│   └── src/
│       └── main.c       # SPI初期化、LCD初期化、カラーバー/全画面塗りつぶしデモ
└── lcd_lvgl/            # プロジェクト2: ST7789上でLVGL v9 GUIデモ
    └── src/
        ├── main.c       # LVGL初期化、ディスプレイ設定、アニメーションバーデモ
        ├── lv_conf.h    # LVGL設定ファイル（v9.6.0-dev）
        ├── lvgl.h       # LVGLマスターヘッダ
        ├── lv_version.h # LVGLバージョンヘッダ
        └── src/         # LVGL v9 ソースツリー（core、widgets、fonts など）
```

## プロジェクト1 — `lcd_spi_ps`（生SPIドライバ）

ST7789を直接制御するミニマルなベアメタルアプリケーションです。

### 機能

- EMIO経由のCS / CD / RES GPIOの初期化
- `XSpiPs`を使用したPS SPIの初期化
- ST7789レジスタ初期化シーケンスの送信
- フレームバッファ（`frame_buffer[240][240]`）によるバルクSPI転送
- **カラーバー**デモ：8色の縦縞表示
- **全画面塗りつぶし**デモ：8色を順番に表示

### 主要な関数

| 関数 | 説明 |
|------|------|
| `Lcd_Gpio_Init()` | CS / CD / RES 用 EMIO GPIO ピンを初期化 |
| `Lcd_Spi_Init()` | PS SPI0をマスターモード（Mode 3）で設定 |
| `Lcd_Init()` | ST7789の初期化コマンドシーケンスを送信 |
| `Address_set(x1,y1,x2,y2)` | 描画ウィンドウ（列・行アドレス）を設定 |
| `LCD_ColorBar_2D()` | バルクSPI転送で8色縦縞を描画 |
| `LCD_FillScreen(clr)` | 画面全体を単色で塗りつぶす |

## プロジェクト2 — `lcd_lvgl`（LVGL v9 GUIデモ）

同じSPIドライバの上に**LVGL v9**オープンソースGUIライブラリを追加したプロジェクトです。

### 機能

- LVGL v9（`lv_conf.h` で v9.6.0-dev 向けに設定済み）
- 10行分のラインバッファ（`lv_buf1`）によるパーシャルレンダリングモード
- カスタムフラッシュコールバック（`lvgl_flush_cb`）：行単位でPS SPIに書き込む
- アニメーション付きデモUI：
  - ネイビー背景
  - タイトルラベル（"LVGL v9 Demo"）
  - アニメーションプログレスバー（0→100→0、50 msタイマー）
  - 色が変化するセンターボックス（RGBカラー補間）
  - 数値ラベル

### 主要な関数 / コールバック

| 関数 | 説明 |
|------|------|
| `lv_init_display()` | LVGLディスプレイを作成し、フラッシュコールバックとパーシャルバッファを設定 |
| `lvgl_flush_cb()` | レンダリングされた各行をSPI経由でST7789に転送 |
| `lv_demo_setup()` | デモUI（背景、バー、ボックス、ラベル、タイマー）を構築 |
| `lv_demo_timer_cb()` | 50 msタイマー：バー値、ボックス色、ラベルを更新 |

## 使い方

### 前提条件

- **Vivado 2023.2** — PS SPI0とEMIO GPIOを公開するハードウェアプラットフォーム（XSA）の生成に使用
- **Vitis Classic 2023.2** — アプリケーションのビルドと実行に使用
- アダプタボード経由でST7789 240×240 LCDを接続したEBAZ4205ボード

### 手順

1. **Vitis Classic**を起動し、新しいワークスペースを作成する。
2. `05_Zynq7000` VivadoプロジェクトのXSAファイルから**プラットフォームプロジェクト**を作成する。
3. **アプリケーションプロジェクト**を作成する：
   - 生ドライバの場合：`lcd_spi_ps`のソース（`src/main.c`）を選択。
   - LVGLデモの場合：`lcd_lvgl`のソース（`src/`以下の全ファイル）を選択。
4. アプリケーションのBSP / プロジェクト設定に**インクルードパス**を追加する：
   - `lcd_lvgl/src`（`lv_conf.h`、`lvgl.h`用）
   - `lcd_lvgl/src/src`（LVGLソースルート）
5. プロジェクトを**ビルド**する（右クリック → Build）。
6. JTAGで**実行 / デバッグ**するか、SDカードに書き込む。
7. **115200 bps**のシリアルターミナルを開いてログ出力を確認する。

### 期待されるUART出力

**lcd_spi_ps**:

```text
EBAZ4205 LCD Test with PS SPI
COLOR BAR
Fill Screen Color=0
Fill Screen Color=1
...
```

**lcd_lvgl**:

```text
EBAZ4205 LCD Test with PS SPI
LVGL initialized successfully!
```

## 注意事項

- ST7789はRGB565ピクセルに**ビッグエンディアン**のバイト順を使用します。
  両プロジェクトとも、ピクセルデータ送信前にバイトスワップを行っています。
- CSはEMIO GPIO経由で手動制御しています（`XSpiPs`に`FORCE_SSELECT`オプションを設定）。
- `lcd_lvgl/src/src/`以下のLVGLソースツリーはアップストリームのLVGL v9ライブラリです。
  変更は`lv_conf.h`のみで行ってください。

## 参考資料

- [ST7789 データシート](../../../documents/adapter/ST7789.pdf)
- [EBAZ4205 回路図](../../../documents/EBAZ4205/EBAZ4205-SCH.pdf)
- [アダプタボード 回路図](../../../documents/adapter/adapter_board_SCH.pdf)
- [LVGL v9 ドキュメント](https://docs.lvgl.io/9/)
