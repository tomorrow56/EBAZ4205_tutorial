/* AXI4-Lite register block for HDMI control
 * Register Map:
 *   0x00: Control Register (R/W)
 *         [1:0] - timing_sel (0=VGA 640x480, 1=480p 720x480, 2=720p 1280x720)
 *         [5:2] - pattern_sel (0..9: color bars/checker/grad/crosshatch/border/RGB/vgrad/walking/SMPTE/zone)
 *         Writing a new timing_sel value triggers MMCM reconfiguration.
 *   0x04: Status Register (read-only)
 *         [0]   - locked (MMCM lock status)
 *         [1]   - reconfig_busy (1 while MMCM reconfiguration in progress)
 */

module axi_hdmi_ctrl #(
    parameter C_S_AXI_DATA_WIDTH = 32,
    parameter C_S_AXI_ADDR_WIDTH = 4
)(
    // AXI4-Lite interface
    input  wire                                 S_AXI_ACLK,
    input  wire                                 S_AXI_ARESETN,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0]       S_AXI_AWADDR,
    input  wire [2:0]                           S_AXI_AWPROT,
    input  wire                                 S_AXI_AWVALID,
    output wire                                 S_AXI_AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1:0]       S_AXI_WDATA,
    input  wire [(C_S_AXI_DATA_WIDTH/8)-1:0]   S_AXI_WSTRB,
    input  wire                                 S_AXI_WVALID,
    output wire                                 S_AXI_WREADY,
    output wire [1:0]                           S_AXI_BRESP,
    output wire                                 S_AXI_BVALID,
    input  wire                                 S_AXI_BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0]       S_AXI_ARADDR,
    input  wire [2:0]                           S_AXI_ARPROT,
    input  wire                                 S_AXI_ARVALID,
    output wire                                 S_AXI_ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1:0]       S_AXI_RDATA,
    output wire [1:0]                           S_AXI_RRESP,
    output wire                                 S_AXI_RVALID,
    input  wire                                 S_AXI_RREADY,
    
    // Control outputs (active in SYSCLK domain, synchronized where needed)
    input  wire                                 PCK,
    input  wire                                 VGA_VS,
    input  wire                                 locked,
    input  wire                                 reconfig_done,
    output reg  [1:0]                           timing_sel_sync,
    output reg  [3:0]                           pattern_sel_sync,
    output reg                                  reconfig_req,
    output wire [1:0]                           timing_sel_imm  // immediate (no VSync latch)
);

// AXI write state machine
reg axi_awready;
reg axi_wready;
reg [1:0] axi_bresp;
reg axi_bvalid;
reg [C_S_AXI_ADDR_WIDTH-1:0] axi_awaddr;

// AXI read state machine
reg axi_arready;
reg [C_S_AXI_DATA_WIDTH-1:0] axi_rdata;
reg [1:0] axi_rresp;
reg axi_rvalid;
reg [C_S_AXI_ADDR_WIDTH-1:0] axi_araddr;

// Registers
reg [1:0] timing_sel_reg;
reg [3:0] pattern_sel_reg;
reg [1:0] timing_sel_prev;   // previous value for change detection
reg       reconfig_busy;

assign timing_sel_imm = timing_sel_reg;

// AXI outputs
assign S_AXI_AWREADY = axi_awready;
assign S_AXI_WREADY  = axi_wready;
assign S_AXI_BRESP   = axi_bresp;
assign S_AXI_BVALID  = axi_bvalid;
assign S_AXI_ARREADY = axi_arready;
assign S_AXI_RDATA   = axi_rdata;
assign S_AXI_RRESP   = axi_rresp;
assign S_AXI_RVALID  = axi_rvalid;

// AXI write address ready
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN)
        axi_awready <= 1'b0;
    else if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID)
        axi_awready <= 1'b1;
    else
        axi_awready <= 1'b0;
end

// AXI write address capture
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN)
        axi_awaddr <= 0;
    else if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID)
        axi_awaddr <= S_AXI_AWADDR;
end

// AXI write data ready
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN)
        axi_wready <= 1'b0;
    else if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID)
        axi_wready <= 1'b1;
    else
        axi_wready <= 1'b0;
end

// Register write logic
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN) begin
        timing_sel_reg <= 2'b00;
        pattern_sel_reg <= 4'b0000;
    end else if (axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID) begin
        case (axi_awaddr[3:2])
            2'h0: begin  // Control register at 0x00
                if (S_AXI_WSTRB[0]) begin
                    timing_sel_reg <= S_AXI_WDATA[1:0];
                    pattern_sel_reg <= S_AXI_WDATA[5:2];
                end
            end
        endcase
    end
end

// MMCM reconfiguration request generation
// Detect timing_sel change and issue a one-cycle pulse
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN) begin
        timing_sel_prev <= 2'b00;
        reconfig_req <= 1'b0;
        reconfig_busy <= 1'b0;
    end else begin
        timing_sel_prev <= timing_sel_reg;
        reconfig_req <= 1'b0;

        if (timing_sel_reg != timing_sel_prev) begin
            reconfig_req <= 1'b1;
            reconfig_busy <= 1'b1;
        end

        if (reconfig_done)
            reconfig_busy <= 1'b0;
    end
end

// AXI write response
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN) begin
        axi_bvalid <= 1'b0;
        axi_bresp <= 2'b0;
    end else if (axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID && ~axi_bvalid) begin
        axi_bvalid <= 1'b1;
        axi_bresp <= 2'b0;
    end else if (S_AXI_BREADY && axi_bvalid) begin
        axi_bvalid <= 1'b0;
    end
end

// AXI read address ready
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN)
        axi_arready <= 1'b0;
    else if (~axi_arready && S_AXI_ARVALID)
        axi_arready <= 1'b1;
    else
        axi_arready <= 1'b0;
end

// AXI read address capture
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN)
        axi_araddr <= 0;
    else if (~axi_arready && S_AXI_ARVALID)
        axi_araddr <= S_AXI_ARADDR;
end

// AXI read data
always @(posedge S_AXI_ACLK) begin
    if (!S_AXI_ARESETN) begin
        axi_rvalid <= 1'b0;
        axi_rresp <= 2'b0;
    end else if (axi_arready && S_AXI_ARVALID && ~axi_rvalid) begin
        axi_rvalid <= 1'b1;
        axi_rresp <= 2'b0;
        case (axi_araddr[3:2])
            2'h0: axi_rdata <= {26'b0, pattern_sel_reg, timing_sel_reg};
            2'h1: axi_rdata <= {30'b0, reconfig_busy, locked};
            default: axi_rdata <= 32'h0;
        endcase
    end else if (axi_rvalid && S_AXI_RREADY) begin
        axi_rvalid <= 1'b0;
    end
end

// Clock domain crossing: AXI clock to PCK domain
// Double-flop synchronizer
reg [1:0] timing_sel_meta, timing_sel_sync_ff;
reg [3:0] pattern_sel_meta, pattern_sel_sync_ff;

always @(posedge PCK) begin
    timing_sel_meta <= timing_sel_reg;
    timing_sel_sync_ff <= timing_sel_meta;
    pattern_sel_meta <= pattern_sel_reg;
    pattern_sel_sync_ff <= pattern_sel_meta;
end

// VSync boundary latch to prevent tearing
reg vsync_prev;
always @(posedge PCK) begin
    vsync_prev <= VGA_VS;
    
    // Latch on rising edge of VSync (start of new frame)
    if (!vsync_prev && VGA_VS) begin
        timing_sel_sync <= timing_sel_sync_ff;
        pattern_sel_sync <= pattern_sel_sync_ff;
    end
end

// Initialize to known values on reset or startup
initial begin
    timing_sel_sync = 2'b00;     // VGA timing
    pattern_sel_sync = 4'b0000;  // Color bars
    timing_sel_meta = 2'b00;     // VGA timing
    timing_sel_sync_ff = 2'b00;  // VGA timing
    pattern_sel_meta = 4'b0000;  // Color bars
    pattern_sel_sync_ff = 4'b0000; // Color bars
    reconfig_busy = 1'b0;
end

endmodule
