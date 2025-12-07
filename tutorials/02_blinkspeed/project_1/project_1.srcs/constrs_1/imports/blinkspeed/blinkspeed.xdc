## EBAZ2405 constraints file
## chapter: 2
## project: blinkspeed

#Clock signal
set_property -dict { PACKAGE_PIN N18   IOSTANDARD LVCMOS33 } [get_ports { CLK }];
create_clock -add -name sys_clk_pin -period 30.00 -waveform {0 4} [get_ports { CLK }];

#Reset
set_property -dict { PACKAGE_PIN V13   IOSTANDARD LVCMOS33 } [get_ports { RST }]; # BTN[1]

# RGB LEDs
set_property -dict { PACKAGE_PIN W14   IOSTANDARD LVCMOS33 } [get_ports { LED_RGB[2]  }]; # Red
set_property -dict { PACKAGE_PIN W13   IOSTANDARD LVCMOS33 } [get_ports { LED_RGB[1]  }]; # Green
set_property -dict { PACKAGE_PIN U12   IOSTANDARD LVCMOS33 } [get_ports { LED_RGB[0]  }]; # Blue
#set_property -dict { PACKAGE_PIN M15   IOSTANDARD LVCMOS33 } [get_ports { LED_RGB2[2] }]; # Red
#set_property -dict { PACKAGE_PIN L14   IOSTANDARD LVCMOS33 } [get_ports { LED_RGB2[1] }]; # Green
#set_property -dict { PACKAGE_PIN G14   IOSTANDARD LVCMOS33 } [get_ports { LED_RGB2[0] }]; # Blue

# Buttons
set_property -dict { PACKAGE_PIN V15   IOSTANDARD LVCMOS33 } [get_ports { BTN[0] }];
#set_property -dict { PACKAGE_PIN D19   IOSTANDARD LVCMOS33 } [get_ports { BTN[1] }];

##Pmod VGA

## Pmod Header JA
#set_property -dict { PACKAGE_PIN Y18   IOSTANDARD LVCMOS33 } [get_ports { VGA_R[0] }]; #ja_p[1]
#set_property -dict { PACKAGE_PIN Y19   IOSTANDARD LVCMOS33 } [get_ports { VGA_R[1] }]; #ja_n[1]
#set_property -dict { PACKAGE_PIN Y16   IOSTANDARD LVCMOS33 } [get_ports { VGA_R[2] }]; #ja_p[2]
#set_property -dict { PACKAGE_PIN Y17   IOSTANDARD LVCMOS33 } [get_ports { VGA_R[3] }]; #ja_n[2]
#set_property -dict { PACKAGE_PIN U18   IOSTANDARD LVCMOS33 } [get_ports { VGA_B[0] }]; #ja_p[3]
#set_property -dict { PACKAGE_PIN U19   IOSTANDARD LVCMOS33 } [get_ports { VGA_B[1] }]; #ja_n[3]
#set_property -dict { PACKAGE_PIN W18   IOSTANDARD LVCMOS33 } [get_ports { VGA_B[2] }]; #ja_p[4]
#set_property -dict { PACKAGE_PIN W19   IOSTANDARD LVCMOS33 } [get_ports { VGA_B[3] }]; #ja_n[4]

## Pmod Header JB
#set_property -dict { PACKAGE_PIN W14   IOSTANDARD LVCMOS33 } [get_ports { VGA_G[0] }]; #jb_p[1]
#set_property -dict { PACKAGE_PIN Y14   IOSTANDARD LVCMOS33 } [get_ports { VGA_G[1] }]; #jb_n[1]
#set_property -dict { PACKAGE_PIN T11   IOSTANDARD LVCMOS33 } [get_ports { VGA_G[2] }]; #jb_p[2]
#set_property -dict { PACKAGE_PIN T10   IOSTANDARD LVCMOS33 } [get_ports { VGA_G[3] }]; #jb_n[2]
#set_property -dict { PACKAGE_PIN V16   IOSTANDARD LVCMOS33 } [get_ports { VGA_HS }];   #jb_p[3]
#set_property -dict { PACKAGE_PIN W16   IOSTANDARD LVCMOS33 } [get_ports { VGA_VS }];   #jb_n[3]

## false path
#set_clock_groups -asynchronous -group [get_clocks clk_fpga_0] \
#    -group [get_clocks -include_generated_clocks clk_fpga_0 -filter {NAME !~ clk_fpga_0}]
