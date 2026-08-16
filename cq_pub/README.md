# CQ Publication Materials

English | [日本語](README_JP.md)

This folder contains project files and archives used for CQ publication
articles about the EBAZ4205 board.

## Overview

The `cq_pub/` directory holds Vivado / Vitis project archives, source
files, and exported hardware files for each article installment. Each
numbered subfolder corresponds to a different project or stage in the
article series.

## Directory Structure

```
cq_pub/
├── 01_led/                         # LED blink project
│   ├── led_project_01.xpr.zip      # Vivado project archive
│   ├── pkg/                        # Packaging scripts and constraints
│   │   ├── bd.tcl
│   │   ├── constrs_1.xdc
│   │   └── setup.tcl
│   └── sim/                        # Simulation files
│       ├── tb_led_blink.sv
│       └── tb_led_blink_behav.wcfg
├── 02_z7000_ps/                    # Zynq7000 PS project
│   ├── project_zynq7000_all.xpr.zip # Vivado project archive
│   └── files/                      # Exported hardware files
│       ├── EBAZ4205.xdc
│       ├── Zynq7000_all_wrapper.xsa
│       └── ps_setting.tcl
├── 03_vitis_classic_z7000_ps/      # Vitis Classic PS applications
│   ├── vitis_EBAZ4205_ps_test_export_archive.ide.zip
│   └── files/
│       ├── Zynq7000_all_wrapper.xsa
│       ├── lcd_ext_ps/
│       │   └── main.c
│       └── led_ext/
│           └── main.c
├── 04_hdmi/                        # HDMI output projects
│   ├── PL_HDMI/                    # PL-only HDMI color bar
│   │   ├── vivado_PL_HDMI.xpr.zip
│   │   └── files/
│   │       ├── PL_HDMI.xdc
│   │       ├── PL_HDMI_wrapper.v
│   │       ├── color_bar.v
│   │       └── rgb2dvi/
│   └── hdmi_axi/                   # AXI4-Lite controlled HDMI pattern generator
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
├── 05_petalinux/                   # PetaLinux project for EBAZ4205
│   ├── project_zynq7000_plnx.xpr.zip # Vivado project archive
│   ├── bsp/                        # BSP package files
│   │   ├── README_BSP.md
│   │   ├── device-tree.bbappend
│   │   ├── ebaz4205_plnx_2024.2_tools.bsp
│   │   └── petalinux-package
│   ├── commands.md                 # PetaLinux command list
│   ├── link.md                     # Reference links
│   └── files/                      # Exported hardware and examples
│       ├── bootgen.bif
│       ├── system-user.dtsi
│       ├── zynq7000_plnx.xsa
│       └── example/
│           ├── axi_gpio/
│           ├── button.sh
│           ├── fpga_load.sh
│           ├── hello/
│           ├── hello.sh
│           └── led.sh
├── ebaz4205_merged.xdc             # Merged constraint file
└── ebaz4205_pin_assign.csv         # Pin assignment CSV
```

## Contents

### 01_led — LED Blink Project

A simple PL-only LED blinking project for the first article.

- `led_project_01.xpr.zip` — Vivado project archive
- `pkg/` — Packaging scripts and constraints
  - `bd.tcl` — Block design Tcl script
  - `constrs_1.xdc` — Constraint file
  - `setup.tcl` — Project setup script
- `sim/` — Simulation testbench
  - `tb_led_blink.sv` — Verilog testbench
  - `tb_led_blink_behav.wcfg` — Behavioral simulation waveform config

### 02_z7000_ps — Zynq7000 PS Project

A Zynq PS hardware project for the EBAZ4205.

- `project_zynq7000_all.xpr.zip` — Vivado project archive
- `files/`
  - `EBAZ4205.xdc` — Pin constraints for the board
  - `Zynq7000_all_wrapper.xsa` — Exported hardware hand-off file
  - `ps_setting.tcl` — Zynq PS configuration Tcl script

### 03_vitis_classic_z7000_ps — Vitis Classic PS Applications

Vitis Classic workspace archive and sample source files for Zynq PS applications.

- `vitis_EBAZ4205_ps_test_export_archive.ide.zip` — Vitis workspace archive
- `files/`
  - `Zynq7000_all_wrapper.xsa` — Hardware export
  - `lcd_ext_ps/main.c` — LCD external control sample
  - `led_ext/main.c` — LED external control sample

### 04_hdmi — HDMI Output Projects

HDMI output projects for the EBAZ4205.

#### PL_HDMI

PL-only HDMI color-bar project.

- `vivado_PL_HDMI.xpr.zip` — Vivado project archive
- `files/`
  - `PL_HDMI.xdc` — Pin / clock constraints
  - `PL_HDMI_wrapper.v` — Top-level wrapper
  - `color_bar.v` — Color bar generator
  - `rgb2dvi/` — Digilent rgb2dvi IP sources

#### hdmi_axi

AXI4-Lite controlled HDMI pattern generator with PS integration.

- `vivado_hdmi_axi.xpr.zip` — Vivado project archive
- `vivado_hdmi_axi/files/`
  - `hdmi_axi_720p.xdc` — 720p constraints
  - `hdmi_axi_ps.tcl` — PS configuration script
  - `hdmi_axi_src_720p/` — RTL sources
  - `system_wrapper.xsa` — Hardware export
- `vitis_hdmi_axi/`
  - `vitis_hdmi_axi_archive.ide.zip` — Vitis workspace archive
  - `files/`
    - `hdmi_axi.c` / `hdmi_axi.h` — AXI driver
    - `main.c` — Application entry

### 05_petalinux — PetaLinux Project for EBAZ4205

A PetaLinux 2024.2 project for the EBAZ4205.
It includes the Vivado hardware design, a custom BSP, build and package
command notes, reference links, and example shell scripts / applications.

- `project_zynq7000_plnx.xpr.zip` — Vivado project archive
- `bsp/`
  - `README_BSP.md` — How to generate a BSP from the PetaLinux project
  - `device-tree.bbappend` — Device-tree recipe patch
  - `ebaz4205_plnx_2024.2_tools.bsp` — Pre-built BSP
  - `petalinux-package` — Patched PetaLinux package script
- `commands.md` — Command list for PetaLinux setup, build, SD card write,
  and GPIO / FPGA test
- `link.md` — Reference links for Vivado, Vitis, PetaLinux, and related
  articles
- `files/`
  - `zynq7000_plnx.xsa` — Exported hardware hand-off
  - `bootgen.bif` — Boot image generation BIF file
  - `system-user.dtsi` — Custom device-tree overlay
  - `example/`
    - `axi_gpio/` — AXI GPIO sample application
    - `hello/` — Hello World sample application
    - `fpga_load.sh` — FPGA bitstream loader script
    - `led.sh` / `button.sh` / `hello.sh` — GPIO and UART test scripts

## Common Files

- `ebaz4205_merged.xdc` — Merged constraint file for the EBAZ4205 board
- `ebaz4205_pin_assign.csv` — Pin assignment CSV

## Note

Project archives (`.xpr.zip` and `.ide.zip`) are Vivado / Vitis exported
projects. Use the corresponding Xilinx tools to import them.
