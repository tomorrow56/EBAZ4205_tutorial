/* Triple Pixel Clock Generator with DRP-based runtime reconfiguration
 * Uses Xilinx mmcm_drp module (from Clocking Wizard IP) for DRP sequencing.
 * Generates PCK (1x) and SerialClk (5x) for rgb2dvi external clock mode.
 *
 * Input: 100 MHz system clock
 * Mode 0 (VGA  640x480@60):  PCK=25.179 MHz, 5x=125.893 MHz
 * Mode 1 (480p 720x480@60):  PCK=27.000 MHz, 5x=135.000 MHz
 * Mode 2 (720p 1280x720@60): PCK=74.250 MHz, 5x=371.250 MHz
 *
 * MMCM parameters per mode (from Vivado Clocking Wizard):
 *   VGA:  DIVCLK=2, M=17.625 (MULT=17,FRAC=625) CLKOUT0=35 CLKOUT1=7  VCO=881.25
 *   480p: DIVCLK=5, M=47.250 (MULT=47,FRAC=250) CLKOUT0=35 CLKOUT1=7  VCO=945.00
 *   720p: DIVCLK=5, M=37.125 (MULT=37,FRAC=125) CLKOUT0=10 CLKOUT1=2  VCO=742.50
 */

module pckgen_triple (
    input           SYSCLK,         // 100 MHz
    input   [1:0]   timing_sel,     // 0=VGA, 1=480p, 2=720p
    input           reconfig_req,   // pulse to request reconfiguration
    output          PCK,            // pixel clock (1x)
    output          SerialClk,      // serial clock (5x)
    output          locked,
    output  reg     reconfig_done   // pulse when reconfiguration complete
);

// =========================================================================
// Mode parameter ROM
// =========================================================================
// High-level parameters fed to mmcm_drp S2 ports for dynamic reconfig.
// Values extracted from Vivado Clocking Wizard generated *_clk_wiz_drp.vhd.

reg  [7:0]  s2_clkfbout_mult;
reg  [9:0]  s2_clkfbout_frac;
reg         s2_clkfbout_frac_en;
reg  [7:0]  s2_divclk_divide;
reg  [7:0]  s2_clkout0_divide;
reg  [9:0]  s2_clkout0_frac;
reg         s2_clkout0_frac_en;
reg  [7:0]  s2_clkout1_divide;

always @(*) begin
    case (timing_sel)
        2'd0: begin  // VGA 640x480@60  M=17.625, VCO=881.25
            s2_clkfbout_mult    = 8'd17;
            s2_clkfbout_frac    = 10'd625;
            s2_clkfbout_frac_en = 1'b1;
            s2_divclk_divide    = 8'd2;
            s2_clkout0_divide   = 8'd35;
            s2_clkout0_frac     = 10'd0;
            s2_clkout0_frac_en  = 1'b0;
            s2_clkout1_divide   = 8'd7;
        end
        2'd1: begin  // 480p 720x480@60  M=47.250, VCO=945.00
            s2_clkfbout_mult    = 8'd47;
            s2_clkfbout_frac    = 10'd250;
            s2_clkfbout_frac_en = 1'b1;
            s2_divclk_divide    = 8'd5;
            s2_clkout0_divide   = 8'd35;
            s2_clkout0_frac     = 10'd0;
            s2_clkout0_frac_en  = 1'b0;
            s2_clkout1_divide   = 8'd7;
        end
        2'd2: begin  // 720p 1280x720@60  M=37.125, VCO=742.50
            s2_clkfbout_mult    = 8'd37;
            s2_clkfbout_frac    = 10'd125;
            s2_clkfbout_frac_en = 1'b1;
            s2_divclk_divide    = 8'd5;
            s2_clkout0_divide   = 8'd10;
            s2_clkout0_frac     = 10'd0;
            s2_clkout0_frac_en  = 1'b0;
            s2_clkout1_divide   = 8'd2;
        end
        default: begin  // default = VGA
            s2_clkfbout_mult    = 8'd17;
            s2_clkfbout_frac    = 10'd625;
            s2_clkfbout_frac_en = 1'b1;
            s2_divclk_divide    = 8'd2;
            s2_clkout0_divide   = 8'd35;
            s2_clkout0_frac     = 10'd0;
            s2_clkout0_frac_en  = 1'b0;
            s2_clkout1_divide   = 8'd7;
        end
    endcase
end

// =========================================================================
// Reconfiguration control state machine
// =========================================================================
// mmcm_drp uses: SADDR=0 -> State1 (initial), SADDR=1 -> State2 (dynamic)
// We always use SADDR=1 (State2) for runtime reconfiguration.
// Sequence: assert LOAD+SEN for 1 cycle -> wait SRDY -> done

localparam S_BOOT_WAIT  = 3'd0;  // wait for mmcm_drp initial config + lock
localparam S_IDLE       = 3'd1;
localparam S_LOAD       = 3'd2;  // assert LOAD (latch S2 params into RAM)
localparam S_SEN        = 3'd3;  // assert SEN (start DRP sequence)
localparam S_WAIT_SRDY  = 3'd4;  // wait for mmcm_drp to finish + MMCM relock
localparam S_DONE       = 3'd5;

reg [2:0]  state;
reg        load_pulse;
reg        sen_pulse;
wire       srdy;
wire       mmcm_locked;

assign locked = mmcm_locked;

initial begin
    state         = S_BOOT_WAIT;
    load_pulse    = 1'b0;
    sen_pulse     = 1'b0;
    reconfig_done = 1'b0;
end

always @(posedge SYSCLK) begin
    load_pulse    <= 1'b0;
    sen_pulse     <= 1'b0;
    reconfig_done <= 1'b0;

    case (state)
        S_BOOT_WAIT: begin
            // Wait for mmcm_drp to complete State1 reconfig and MMCM to lock
            if (srdy) begin
                state <= S_IDLE;
            end
        end

        S_IDLE: begin
            if (reconfig_req) begin
                state <= S_LOAD;
            end
        end

        S_LOAD: begin
            // Assert LOAD to latch S2 parameters into mmcm_drp RAM
            load_pulse <= 1'b1;
            state      <= S_SEN;
        end

        S_SEN: begin
            // Assert SEN one cycle after LOAD to start DRP reconfig
            sen_pulse  <= 1'b1;
            state      <= S_WAIT_SRDY;
        end

        S_WAIT_SRDY: begin
            // Wait for mmcm_drp to complete reconfiguration and MMCM to lock
            if (srdy) begin
                state <= S_DONE;
            end
        end

        S_DONE: begin
            reconfig_done <= 1'b1;
            state <= S_IDLE;
        end

        default: state <= S_BOOT_WAIT;
    endcase
end

// =========================================================================
// DRP interface wires
// =========================================================================
wire        drp_den;
wire        drp_dwe;
wire [6:0]  drp_daddr;
wire [15:0] drp_di;
wire [15:0] drp_do;
wire        drp_drdy;
wire        drp_dclk;
wire        mmcm_rst;

// =========================================================================
// MMCME2_ADV instance
// =========================================================================
// Initial configuration matches VGA mode (State1 of mmcm_drp)
wire iPCK, iSerialClk;
wire CLKFBOUT, CLKFBIN;

MMCME2_ADV #(
    .BANDWIDTH          ("OPTIMIZED"),
    .CLKIN1_PERIOD      (10.0),           // 100 MHz
    .DIVCLK_DIVIDE      (2),
    .CLKFBOUT_MULT_F    (17.625),         // VCO = 881.25 MHz (VGA default)
    .CLKFBOUT_PHASE     (0.0),
    .CLKOUT0_DIVIDE_F   (35.0),           // 25.179 MHz (VGA default)
    .CLKOUT0_DUTY_CYCLE (0.5),
    .CLKOUT0_PHASE      (0.0),
    .CLKOUT1_DIVIDE     (7),              // 125.893 MHz (VGA 5x default)
    .CLKOUT1_DUTY_CYCLE (0.5),
    .CLKOUT1_PHASE      (0.0),
    .STARTUP_WAIT       ("FALSE"),
    .REF_JITTER1        (0.01)
) MMCM_RECONFIG (
    .CLKIN1     (SYSCLK),
    .CLKIN2     (1'b0),
    .CLKINSEL   (1'b1),
    .CLKFBIN    (CLKFBIN),
    .CLKFBOUT   (CLKFBOUT),
    .CLKFBOUTB  (),
    .CLKOUT0    (iPCK),
    .CLKOUT0B   (),
    .CLKOUT1    (iSerialClk),
    .CLKOUT1B   (),
    .CLKOUT2    (), .CLKOUT2B   (),
    .CLKOUT3    (), .CLKOUT3B   (),
    .CLKOUT4    (), .CLKOUT5    (), .CLKOUT6    (),
    .LOCKED     (mmcm_locked),
    .PWRDWN     (1'b0),
    .RST        (mmcm_rst),
    // DRP interface (directly controlled by mmcm_drp)
    .DCLK       (drp_dclk),
    .DADDR      (drp_daddr),
    .DEN        (drp_den),
    .DWE        (drp_dwe),
    .DI         (drp_di),
    .DO         (drp_do),
    .DRDY       (drp_drdy)
);

// Feedback buffer
BUFG BUFG_FB (.I(CLKFBOUT), .O(CLKFBIN));

// Output clock buffers
BUFG BUFG_PCK (.I(iPCK), .O(PCK));
BUFG BUFG_5X  (.I(iSerialClk), .O(SerialClk));

// =========================================================================
// Xilinx mmcm_drp instance (read-modify-write DRP controller)
// =========================================================================
// State1 (S1_*) = VGA initial configuration (matches MMCME2_ADV params above)
// State2 (S2_*) = dynamic reconfiguration from ports
mmcm_drp #(
    // State 1: VGA 640x480@60 (power-on default)
    .S1_CLKFBOUT_MULT    (17),
    .S1_CLKFBOUT_PHASE   (0),
    .S1_CLKFBOUT_FRAC    (1649),
    .S1_CLKFBOUT_FRAC_EN (1),
    .S1_BANDWIDTH         ("OPTIMIZED"),
    .S1_DIVCLK_DIVIDE    (2),
    .S1_CLKOUT0_DIVIDE   (35),
    .S1_CLKOUT0_PHASE    (0),
    .S1_CLKOUT0_DUTY     (50000),
    .S1_CLKOUT0_FRAC     (0),
    .S1_CLKOUT0_FRAC_EN  (0),
    .S1_CLKOUT1_DIVIDE   (7),
    .S1_CLKOUT1_PHASE    (0),
    .S1_CLKOUT1_DUTY     (50000),
    .S1_CLKOUT2_DIVIDE   (1),
    .S1_CLKOUT2_PHASE    (0),
    .S1_CLKOUT2_DUTY     (50000),
    .S1_CLKOUT3_DIVIDE   (1),
    .S1_CLKOUT3_PHASE    (0),
    .S1_CLKOUT3_DUTY     (50000),
    .S1_CLKOUT4_DIVIDE   (1),
    .S1_CLKOUT4_PHASE    (0),
    .S1_CLKOUT4_DUTY     (50000),
    .S1_CLKOUT5_DIVIDE   (1),
    .S1_CLKOUT5_PHASE    (0),
    .S1_CLKOUT5_DUTY     (50000),
    .S1_CLKOUT6_DIVIDE   (1),
    .S1_CLKOUT6_PHASE    (0),
    .S1_CLKOUT6_DUTY     (50000)
) mmcm_drp_inst (
    // State 2 dynamic inputs
    .S2_CLKFBOUT_MULT    (s2_clkfbout_mult),
    .S2_CLKFBOUT_FRAC    (s2_clkfbout_frac),
    .S2_CLKFBOUT_FRAC_EN (s2_clkfbout_frac_en),
    .S2_DIVCLK_DIVIDE    (s2_divclk_divide),
    .S2_CLKOUT0_DIVIDE   (s2_clkout0_divide),
    .S2_CLKOUT0_FRAC     (s2_clkout0_frac),
    .S2_CLKOUT0_FRAC_EN  (s2_clkout0_frac_en),
    .S2_CLKOUT1_DIVIDE   (s2_clkout1_divide),
    .S2_CLKOUT2_DIVIDE   (8'd1),
    .S2_CLKOUT3_DIVIDE   (8'd1),
    .S2_CLKOUT4_DIVIDE   (8'd1),
    .S2_CLKOUT5_DIVIDE   (8'd1),
    .S2_CLKOUT6_DIVIDE   (8'd1),
    // Control
    .LOAD                (load_pulse),
    .SADDR               (1'b1),          // always use State2 for reconfig
    .SEN                 (sen_pulse),
    .RST                 (1'b0),
    .SRDY                (srdy),
    .SCLK                (SYSCLK),
    // DRP interface to MMCM
    .DO                  (drp_do),
    .DRDY                (drp_drdy),
    .LOCKED              (mmcm_locked),
    .DWE                 (drp_dwe),
    .DEN                 (drp_den),
    .DADDR               (drp_daddr),
    .DI                  (drp_di),
    .DCLK                (drp_dclk),
    .RST_MMCM_PLL        (mmcm_rst)
);

endmodule
