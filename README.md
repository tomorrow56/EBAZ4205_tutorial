# EBAZ4205 Tutorial

English | [日本語](README_JP.md)

A collection of FPGA/SoC development tutorials using EBAZ4205 (a board with a Zynq-7000 SoC).

## Overview

This repository contains tutorial projects to learn development for the Xilinx Zynq-7000 SoC using the EBAZ4205 board.

### Target Device

- **SoC**: Xilinx Zynq-7000 (xc7z010clg400-1)
- **Board**: EBAZ4205

## Directory Structure

```
EBAZ4205_tutorial/
├── documents/                    # Datasheets, schematics, reference materials, boot images, etc.
│   ├── EBAZ4205/                  # Board materials (schematics, manuals, images, etc.)
│   ├── xilinx_user_guide/         # Official Xilinx User Guides
│   ├── TF_boot_image/             # Boot images
│   ├── ether_phy/                 # Ethernet PHY-related documents
│   ├── adapter/                   # Adapter / verification materials
│   ├── jtag/                      # JTAG-related documents
│   ├── SoC_info.txt               # SoC part number note
│   ├── README.md                  # Documents overview (English)
│   └── README_JP.md               # Documents overview (Japanese)
├── tutorials/                    # Tutorial projects
│   ├── 01_blink/                  # LED blinking (basic)
│   ├── 02_blinkspeed/             # LED blink speed control
│   ├── 03_pattern/                # Pattern display (HDMI output)
│   ├── 04_gradation/              # Gradation display (HDMI output)
│   ├── 05_Zynq7000/               # Zynq PS+PL integrated project
│   ├── 06_hdmi_test/              # HDMI pattern generator with AXI4-Lite control
│   ├── 07_lcd_spi/                # PS-driven ST7789 SPI LCD (raw SPI / LVGL v9)
│   ├── ebaz4205_merged.xdc        # Merged constraint file (XDC)
│   └── ebaz4205_pin_assign.csv    # Pin assignment CSV
├── cq_pub/                       # CQ publication materials
│   ├── 01_led/                    # LED project files
│   ├── 02_z7000_ps/               # Zynq7000 PS project files
│   ├── 03_vitis_classic_z7000_ps/ # Vitis Classic PS project files
│   ├── 04_hdmi/                   # HDMI project files
│   │   ├── PL_HDMI/               # PL-based HDMI output
│   │   └── hdmi_axi/              # AXI4-Lite controlled HDMI project
│   ├── ebaz4205_merged.xdc        # Merged constraint file (XDC)
│   └── ebaz4205_pin_assign.csv    # Pin assignment CSV
└── LICENSE
```

## Tutorial Contents

### 01_blink

A basic PL project to control RGB LED blinking.

- Clock division of the system clock
- LED control using a counter
- Constraint file (XDC) settings

### 02_blinkspeed

Learn how to control the LED blink speed by button input.

- Debounce circuit implementation
- User input handling

### 03_pattern

Display patterns via HDMI output.

- Sync signal generation (syncgen)
- Pixel clock generation (pckgen)
- HDMI output control

### 04_gradation

An HDMI output application using gradation display.

### 05_Zynq7000

A project that integrates the Zynq SoC PS (Processing System) and PL (Programmable Logic).

- Vivado block design with GPIO/UART/Ethernet/Timer peripherals
- Multiple Vitis applications (LED External Test 1/2, lwIP Echo Server, Memory Test, Peripheral Test, PS LCD)
- UART communication and PS-PL integration

### 06_hdmi_test

An HDMI pattern generator with AXI4-Lite register control from the PS.

- AXI4-Lite slave interface for dynamic switching
- Multiple timing presets (VGA, 480p, 720p) and 10 test patterns
- MMCM DRP for runtime clock switching
- Digilent rgb2dvi IP for TMDS/HDMI output

### 07_lcd_spi

PS-driven ST7789 SPI LCD (240x240 px) projects.

- EMIO GPIO and PS SPI0 control
- `lcd_spi_ps`: raw SPI LCD driver (color bar / fill screen demos)
- `lcd_lvgl`: LVGL v9 GUI demo

## CQ Publication Materials

This folder contains project files and archives for CQ publication articles.

- `01_led/` — LED project files
- `02_z7000_ps/` — Zynq7000 PS project files
- `03_vitis_classic_z7000_ps/` — Vitis Classic PS project archive
- `04_hdmi/` — HDMI project files
  - `PL_HDMI/` — PL-based HDMI output
  - `hdmi_axi/` — AXI4-Lite controlled HDMI project

## Development Environment

- **Vivado**: 2024.2
- **Vitis**: 2024.2

## References

The following materials are included in the `documents/` folder:

- EBAZ4205 schematics, manuals, PCB data, Linux image (`documents/EBAZ4205/`)
- Zynq-7000 TRM (Technical Reference Manual) (`documents/xilinx_user_guide/`)
- Boot images (`documents/TF_boot_image/`)
- Ethernet PHY documents (`documents/ether_phy/`)
- SoC part number note (`documents/SoC_info.txt`)
- Documents folder overview (`documents/README.md`, `documents/README_JP.md`)

## Articles

- https://qiita.com/tomorrow56/items/7a6340c04b87f584288a

## Resources

- https://github.com/xjtuecho/EBAZ4205/

## Where to Buy (Reference)

- https://ja.aliexpress.com/item/1005006074065888.html
- SD card slot part: [SHOU HAN MEM2055-00-140-00-A (LCSC C393941)](https://www.lcsc.com/product-detail/C393941.html)

## License

MIT License

Copyright (c) 2025 tomorrow56 A.K.A. ThousanDIY

## Acknowledgements

Some of the code is based on samples from [Cobac.Net](https://www.cobac.net/).
