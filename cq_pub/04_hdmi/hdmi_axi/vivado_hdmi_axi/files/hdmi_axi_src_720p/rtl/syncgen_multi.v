/* Multi-preset sync signal generator with timing_sel control
 * Preset 0: VGA 640x480@60
 * Preset 1: 480p 720x480@60
 * Preset 2: 720p 1280x720@60
 */

module syncgen_multi(
    input               CLK,
    input               RST,
    input       [1:0]   timing_sel,     // 0=VGA, 1=480p, 2=720p (VSync latched, for counters)
    input       [1:0]   timing_sel_imm, // immediate (for MMCM reconfig params)
    input               reconfig_req,   // pulse to trigger MMCM reconfig
    output              PCK,
    output              SerialClk,      // 5x pixel clock for rgb2dvi
    output              locked,
    output              reconfig_done,  // pulse when MMCM reconfig complete
    output  reg         VGA_HS,
    output  reg         VGA_VS,
    output  reg [10:0]  HCNT,
    output  reg [10:0]  VCNT,
    output  reg [7:0]   FCNT           // frame counter (for animated patterns)
);

// Timing parameters for VGA 640x480@60
localparam VGA_HPERIOD = 11'd800;
localparam VGA_HFRONT  = 11'd16;
localparam VGA_HWIDTH  = 11'd96;
localparam VGA_HBACK   = 11'd48;
localparam VGA_VPERIOD = 11'd525;
localparam VGA_VFRONT  = 11'd10;
localparam VGA_VWIDTH  = 11'd2;
localparam VGA_VBACK   = 11'd33;

// Timing parameters for 480p 720x480@60
localparam P480_HPERIOD = 11'd858;
localparam P480_HFRONT  = 11'd16;
localparam P480_HWIDTH  = 11'd62;
localparam P480_HBACK   = 11'd60;
localparam P480_VPERIOD = 11'd525;
localparam P480_VFRONT  = 11'd9;
localparam P480_VWIDTH  = 11'd6;
localparam P480_VBACK   = 11'd30;

// Timing parameters for 720p 1280x720@60
localparam P720_HPERIOD = 11'd1650;
localparam P720_HFRONT  = 11'd110;
localparam P720_HWIDTH  = 11'd40;
localparam P720_HBACK   = 11'd220;
localparam P720_VPERIOD = 11'd750;
localparam P720_VFRONT  = 11'd5;
localparam P720_VWIDTH  = 11'd5;
localparam P720_VBACK   = 11'd20;

// Select timing parameters based on timing_sel
reg [10:0] HPERIOD, HFRONT, HWIDTH, HBACK;
reg [10:0] VPERIOD, VFRONT, VWIDTH, VBACK;
reg        hs_polarity;  // 0=active-low, 1=active-high
reg        vs_polarity;  // 0=active-low, 1=active-high

always @(*) begin
    case (timing_sel)
        2'd1: begin  // 480p
            HPERIOD = P480_HPERIOD; HFRONT = P480_HFRONT;
            HWIDTH  = P480_HWIDTH;  HBACK  = P480_HBACK;
            VPERIOD = P480_VPERIOD; VFRONT = P480_VFRONT;
            VWIDTH  = P480_VWIDTH;  VBACK  = P480_VBACK;
            hs_polarity = 1'b0;  // active-low
            vs_polarity = 1'b0;  // active-low
        end
        2'd2: begin  // 720p
            HPERIOD = P720_HPERIOD; HFRONT = P720_HFRONT;
            HWIDTH  = P720_HWIDTH;  HBACK  = P720_HBACK;
            VPERIOD = P720_VPERIOD; VFRONT = P720_VFRONT;
            VWIDTH  = P720_VWIDTH;  VBACK  = P720_VBACK;
            hs_polarity = 1'b1;  // active-high (CEA-861)
            vs_polarity = 1'b1;  // active-high (CEA-861)
        end
        default: begin  // VGA
            HPERIOD = VGA_HPERIOD; HFRONT = VGA_HFRONT;
            HWIDTH  = VGA_HWIDTH;  HBACK  = VGA_HBACK;
            VPERIOD = VGA_VPERIOD; VFRONT = VGA_VFRONT;
            VWIDTH  = VGA_VWIDTH;  VBACK  = VGA_VBACK;
            hs_polarity = 1'b0;  // active-low
            vs_polarity = 1'b0;  // active-low
        end
    endcase
end

// Triple MMCM pixel clock generator with DRP reconfiguration
pckgen_triple pckgen_triple (
    .SYSCLK(CLK),
    .timing_sel(timing_sel_imm),
    .reconfig_req(reconfig_req),
    .PCK(PCK),
    .SerialClk(SerialClk),
    .locked(locked),
    .reconfig_done(reconfig_done)
);

// Horizontal counter
wire hcntend = (HCNT == HPERIOD - 11'h001);

always @(posedge PCK) begin
    if (RST)
        HCNT <= 11'h000;
    else if (hcntend)
        HCNT <= 11'h000;
    else
        HCNT <= HCNT + 11'h001;
end

// Vertical counter
always @(posedge PCK) begin
    if (RST)
        VCNT <= 11'h000;
    else if (hcntend) begin
        if (VCNT == VPERIOD - 11'h001)
            VCNT <= 11'h000;
        else
            VCNT <= VCNT + 11'h001;
    end
end

// Sync signals
wire [10:0] hsstart = HFRONT - 11'h001;
wire [10:0] hsend   = HFRONT + HWIDTH - 11'h001;
wire [10:0] vsstart = VFRONT;
wire [10:0] vsend   = VFRONT + VWIDTH;

// HS/VS active level depends on polarity setting
// VGA/480p: active-low (idle=1, active=0)
// 720p:     active-high (idle=0, active=1) per CEA-861
always @(posedge PCK) begin
    if (RST)
        VGA_HS <= ~hs_polarity;
    else if (HCNT == hsstart)
        VGA_HS <= hs_polarity;
    else if (HCNT == hsend)
        VGA_HS <= ~hs_polarity;
end

always @(posedge PCK) begin
    if (RST)
        VGA_VS <= ~vs_polarity;
    else if (HCNT == hsstart) begin
        if (VCNT == vsstart)
            VGA_VS <= vs_polarity;
        else if (VCNT == vsend)
            VGA_VS <= ~vs_polarity;
    end
end

// Frame counter (increments at start of each frame)
always @(posedge PCK) begin
    if (RST)
        FCNT <= 8'd0;
    else if (hcntend && VCNT == VPERIOD - 11'h001)
        FCNT <= FCNT + 8'd1;
end

endmodule
