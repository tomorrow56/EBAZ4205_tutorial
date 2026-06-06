/* Multi-pattern test pattern generator with pattern_sel control
 * Pattern 0: Color bars (8 vertical bars)
 * Pattern 1: Checkerboard (32x32)
 * Pattern 2: Horizontal gradation
 * Pattern 3: Crosshatch (64px grid)
 * Pattern 4: Border (1px white frame)
 * Pattern 5: RGB full field (cycle by frame)
 * Pattern 6: Vertical gradation
 * Pattern 7: Walking bar (animated)
 * Pattern 8: SMPTE color bars
 * Pattern 9: Zone plate (concentric circles)
 */

module pattern_multi(
    input               CLK,
    input               RST,
    input       [1:0]   timing_sel,    // 0=VGA, 1=480p, 2=720p (VSync latched)
    input       [1:0]   timing_sel_imm, // immediate (for MMCM reconfig)
    input       [3:0]   pattern_sel,   // 0..9 pattern select
    input               reconfig_req,  // pulse to trigger MMCM reconfig
    output  reg [7:0]   VGA_R, VGA_G, VGA_B,
    output              VGA_HS, VGA_VS,
    output  reg         VGA_DE,
    output              PCK,
    output              SerialClk,     // 5x pixel clock for rgb2dvi
    output              locked,
    output              reconfig_done  // pulse when MMCM reconfig complete
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

always @(*) begin
    case (timing_sel)
        2'd1: begin  // 480p
            HPERIOD = P480_HPERIOD; HFRONT = P480_HFRONT;
            HWIDTH  = P480_HWIDTH;  HBACK  = P480_HBACK;
            VPERIOD = P480_VPERIOD; VFRONT = P480_VFRONT;
            VWIDTH  = P480_VWIDTH;  VBACK  = P480_VBACK;
        end
        2'd2: begin  // 720p
            HPERIOD = P720_HPERIOD; HFRONT = P720_HFRONT;
            HWIDTH  = P720_HWIDTH;  HBACK  = P720_HBACK;
            VPERIOD = P720_VPERIOD; VFRONT = P720_VFRONT;
            VWIDTH  = P720_VWIDTH;  VBACK  = P720_VBACK;
        end
        default: begin  // VGA
            HPERIOD = VGA_HPERIOD; HFRONT = VGA_HFRONT;
            HWIDTH  = VGA_HWIDTH;  HBACK  = VGA_HBACK;
            VPERIOD = VGA_VPERIOD; VFRONT = VGA_VFRONT;
            VWIDTH  = VGA_VWIDTH;  VBACK  = VGA_VBACK;
        end
    endcase
end

// Sync generator
wire [10:0] HCNT, VCNT;

wire [7:0] FCNT;

syncgen_multi syncgen_multi(
    .CLK            (CLK),
    .RST            (RST),
    .timing_sel     (timing_sel),
    .timing_sel_imm (timing_sel_imm),
    .reconfig_req   (reconfig_req),
    .PCK            (PCK),
    .SerialClk      (SerialClk),
    .locked         (locked),
    .reconfig_done  (reconfig_done),
    .VGA_HS         (VGA_HS),
    .VGA_VS         (VGA_VS),
    .HCNT           (HCNT),
    .VCNT           (VCNT),
    .FCNT           (FCNT)
);

// Display area calculation
wire [10:0] HBLANK = HFRONT + HWIDTH + HBACK;
wire [10:0] VBLANK = VFRONT + VWIDTH + VBACK;

wire disp_enable = (VBLANK <= VCNT)
                && (HBLANK - 11'd1 <= HCNT) && (HCNT < HPERIOD - 11'd1);

// Active display coordinates
wire [10:0] disp_x = HCNT - HBLANK + 11'd1;
wire [10:0] disp_y = VCNT - VBLANK;

// Active resolution (depends on timing_sel)
reg [10:0] HACTIVE;
reg [10:0] VACTIVE;
always @(*) begin
    case (timing_sel)
        2'd1:    begin HACTIVE = 11'd720;  VACTIVE = 11'd480; end
        2'd2:    begin HACTIVE = 11'd1280; VACTIVE = 11'd720; end
        default: begin HACTIVE = 11'd640;  VACTIVE = 11'd480; end
    endcase
end

// Pattern generation
reg [7:0] pattern_r, pattern_g, pattern_b;

// Walking bar position (animated)
wire [10:0] walk_pos = (FCNT * 11'd4) % HACTIVE;

// Zone plate: simplified (x^2 + y^2) >> shift
wire [21:0] zp_sum = disp_x * disp_x + disp_y * disp_y;
wire [7:0]  zp_val = zp_sum[12:5];  // adjust bit slice for visible rings

always @(*) begin
    case (pattern_sel)
        4'd0: begin  // Color bars (8 vertical bars)
            case (disp_x * 8 / HACTIVE)
                3'd0: {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'hFF}; // White
                3'd1: {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'h00}; // Yellow
                3'd2: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'hFF, 8'hFF}; // Cyan
                3'd3: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'hFF, 8'h00}; // Green
                3'd4: {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'h00, 8'hFF}; // Magenta
                3'd5: {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'h00, 8'h00}; // Red
                3'd6: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'hFF}; // Blue
                3'd7: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00}; // Black
                default: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
            endcase
        end

        4'd1: begin  // Checkerboard (32x32 pixel squares)
            if ((disp_x[5] ^ disp_y[5]) == 1'b1)
                {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'hFF};
            else
                {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
        end

        4'd2: begin  // Horizontal gradation
            pattern_r = (disp_x * 19'd255) / {8'd0, HACTIVE};
            pattern_g = (disp_x * 19'd255) / {8'd0, HACTIVE};
            pattern_b = (disp_x * 19'd255) / {8'd0, HACTIVE};
        end

        4'd3: begin  // Crosshatch (64px grid lines)
            if ((disp_x[5:0] == 6'd0) || (disp_y[5:0] == 6'd0))
                {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'hFF};
            else
                {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
        end

        4'd4: begin  // Border (1px white frame)
            if (disp_x == 11'd0 || disp_x == HACTIVE - 11'd1 ||
                disp_y == 11'd0 || disp_y == VACTIVE - 11'd1)
                {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'hFF};
            else
                {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
        end

        4'd5: begin  // RGB full field (cycles R->G->B every ~21 frames)
            case (FCNT[6:5])
                2'd0: {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'h00, 8'h00}; // Red
                2'd1: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'hFF, 8'h00}; // Green
                2'd2: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'hFF}; // Blue
                2'd3: {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'hFF}; // White
            endcase
        end

        4'd6: begin  // Vertical gradation (top=black, bottom=white)
            pattern_r = (disp_y * 19'd255) / {8'd0, VACTIVE};
            pattern_g = (disp_y * 19'd255) / {8'd0, VACTIVE};
            pattern_b = (disp_y * 19'd255) / {8'd0, VACTIVE};
        end

        4'd7: begin  // Walking bar (4px white bar moving right)
            if (disp_x >= walk_pos && disp_x < walk_pos + 11'd4)
                {pattern_r, pattern_g, pattern_b} = {8'hFF, 8'hFF, 8'hFF};
            else
                {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
        end

        4'd8: begin  // SMPTE-like color bars
            // Top 67%: standard 7 bars, Bottom 33%: reverse + PLUGE
            if (disp_y < (VACTIVE * 2 / 3)) begin
                // Upper: 7 color bars
                case (disp_x * 7 / HACTIVE)
                    3'd0: {pattern_r, pattern_g, pattern_b} = {8'hC0, 8'hC0, 8'hC0}; // 75% White
                    3'd1: {pattern_r, pattern_g, pattern_b} = {8'hC0, 8'hC0, 8'h00}; // Yellow
                    3'd2: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'hC0, 8'hC0}; // Cyan
                    3'd3: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'hC0, 8'h00}; // Green
                    3'd4: {pattern_r, pattern_g, pattern_b} = {8'hC0, 8'h00, 8'hC0}; // Magenta
                    3'd5: {pattern_r, pattern_g, pattern_b} = {8'hC0, 8'h00, 8'h00}; // Red
                    3'd6: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'hC0}; // Blue
                    default: {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
                endcase
            end else begin
                // Lower: PLUGE-like (black with near-black reference)
                if (disp_x < HACTIVE / 6)
                    {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h30}; // -4% (dark blue hint)
                else if (disp_x < HACTIVE * 2 / 6)
                    {pattern_r, pattern_g, pattern_b} = {8'h10, 8'h10, 8'h10}; // +4%
                else if (disp_x < HACTIVE * 3 / 6)
                    {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00}; // 0% Black
                else if (disp_x < HACTIVE * 4 / 6)
                    {pattern_r, pattern_g, pattern_b} = {8'h08, 8'h08, 8'h08}; // +2%
                else if (disp_x < HACTIVE * 5 / 6)
                    {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00}; // 0% Black
                else
                    {pattern_r, pattern_g, pattern_b} = {8'h20, 8'h20, 8'h20}; // +8%
            end
        end

        4'd9: begin  // Zone plate (concentric circles)
            pattern_r = zp_val;
            pattern_g = zp_val;
            pattern_b = zp_val;
        end

        default: begin  // Black
            {pattern_r, pattern_g, pattern_b} = {8'h00, 8'h00, 8'h00};
        end
    endcase
end

// RGB output
always @(posedge PCK) begin
    if (RST)
        {VGA_R, VGA_G, VGA_B} <= 24'h0;
    else if (disp_enable)
        {VGA_R, VGA_G, VGA_B} <= {pattern_r, pattern_g, pattern_b};
    else
        {VGA_R, VGA_G, VGA_B} <= 24'h0;
end

// Display enable delayed by 1 clock for VGA_DE
always @(posedge PCK) begin
    if (RST)
        VGA_DE <= 1'b0;
    else
        VGA_DE <= disp_enable;
end

endmodule
