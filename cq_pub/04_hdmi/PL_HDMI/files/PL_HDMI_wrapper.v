//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2024.2 (win64) Build 5239630 Fri Nov 08 22:35:27 MST 2024
//Date        : Sat Jun  6 12:54:34 2026
//Host        : MASAO running 64-bit major release  (build 9200)
//Command     : generate_target PL_HDMI_wrapper.bd
//Design      : PL_HDMI_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module PL_HDMI_wrapper
   (TMDS_CLK_n,
    TMDS_CLK_p,
    TMDS_DATA_n,
    TMDS_DATA_p,
    clk);
  output TMDS_CLK_n;
  output TMDS_CLK_p;
  output [2:0]TMDS_DATA_n;
  output [2:0]TMDS_DATA_p;
  input clk;

  wire TMDS_CLK_n;
  wire TMDS_CLK_p;
  wire [2:0]TMDS_DATA_n;
  wire [2:0]TMDS_DATA_p;
  wire clk;

  PL_HDMI PL_HDMI_i
       (.TMDS_CLK_n(TMDS_CLK_n),
        .TMDS_CLK_p(TMDS_CLK_p),
        .TMDS_DATA_n(TMDS_DATA_n),
        .TMDS_DATA_p(TMDS_DATA_p),
        .clk(clk));
endmodule
