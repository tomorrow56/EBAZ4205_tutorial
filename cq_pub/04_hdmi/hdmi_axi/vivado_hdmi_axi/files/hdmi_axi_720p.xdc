# EBAZ4205 constraints file for AXI-controlled HDMI pattern generator
# Project: 06_hdmi_test with AXI4-Lite control
# Note: CLK and RST are provided by PS FCLK_CLK1 and FCLK_RESET1_N (no external pins)

# UART on Type-C
set_property PACKAGE_PIN H16 [get_ports UART_0_rxd]
set_property IOSTANDARD LVCMOS33 [get_ports UART_0_rxd]
set_property PACKAGE_PIN H17 [get_ports UART_0_txd]
set_property IOSTANDARD LVCMOS33 [get_ports UART_0_txd]

# HDMI TX outputs
set_property -dict {PACKAGE_PIN F20 IOSTANDARD TMDS_33} [get_ports HDMI_CLK_N]
set_property -dict {PACKAGE_PIN F19 IOSTANDARD TMDS_33} [get_ports HDMI_CLK_P]
set_property -dict {PACKAGE_PIN D20 IOSTANDARD TMDS_33} [get_ports {HDMI_N[0]}]
set_property -dict {PACKAGE_PIN D19 IOSTANDARD TMDS_33} [get_ports {HDMI_P[0]}]
set_property -dict {PACKAGE_PIN B20 IOSTANDARD TMDS_33} [get_ports {HDMI_N[1]}]
set_property -dict {PACKAGE_PIN C20 IOSTANDARD TMDS_33} [get_ports {HDMI_P[1]}]
set_property -dict {PACKAGE_PIN A20 IOSTANDARD TMDS_33} [get_ports {HDMI_N[2]}]
set_property -dict {PACKAGE_PIN B19 IOSTANDARD TMDS_33} [get_ports {HDMI_P[2]}]

# Clock domain crossing constraints
# MMCM hierarchy: pckgen_triple/MMCM_RECONFIG
#   CLKOUT0 = PCK (pixel clock, DRP-reconfigurable)
#   CLKOUT1 = SerialClk (5x pixel clock, DRP-reconfigurable)

# AXI clock (PS FCLK_CLK0, 100 MHz) to pixel clock domain (CDC handled by double-FF + VSync latch)
set_false_path -from [get_clocks clk_fpga_0] -to [get_clocks -of_objects [get_pins system_i/hdmi_axi/pattern_multi_0/inst/syncgen_multi/pckgen_triple/MMCM_RECONFIG/CLKOUT0]]

# AXI clock to serial clock domain
set_false_path -from [get_clocks clk_fpga_0] -to [get_clocks -of_objects [get_pins system_i/hdmi_axi/pattern_multi_0/inst/syncgen_multi/pckgen_triple/MMCM_RECONFIG/CLKOUT1]]

# Pixel clock domain back to AXI clock domain (status readback)
set_false_path -from [get_clocks -of_objects [get_pins system_i/hdmi_axi/pattern_multi_0/inst/syncgen_multi/pckgen_triple/MMCM_RECONFIG/CLKOUT0]] -to [get_clocks clk_fpga_0]
