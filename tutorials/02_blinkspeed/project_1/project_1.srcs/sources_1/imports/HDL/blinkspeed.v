/* Copyright(C) 2020 Cobac.Net All Rights Reserved. */
/* chapter: 第2章                           */
/* project: blinkspeed                      */
/* outline: LEDの点滅速度をプッシュSWで制御 */
/* modifyed for EBAZ4205                  */

module blinkspeed (
    input               CLK,
    input               RST,
    input       [0:0]   BTN,
    output  reg [2:0]   LED_RGB
);

/* チャタリング除去回路を接続 */
wire btnon;

debounce d0 (.CLK(CLK), .RST(RST), .BTNIN(BTN), .BTNOUT(btnon));

/* 速度設定用カウンタ */
reg [1:0] speed;

always @( posedge CLK ) begin
    if ( RST )
        speed <= 2'h0;
    else if ( btnon )
        speed <= speed + 2'h1;
end

/* システムクロックを分周 */
reg [24:0] cnt25;

always @( posedge CLK ) begin
    if ( RST )
        cnt25 <= 25'h0;
    else
        cnt25 <= cnt25 + 25'h1;
end

/* LED用カウンタのイネーブルを作成 */
reg ledcnten;

always @* begin
    case ( speed )
        2'h0:   ledcnten = (cnt25      ==25'h1ffffff);
        2'h1:   ledcnten = (cnt25[23:0]==24'hffffff);
        2'h2:   ledcnten = (cnt25[22:0]==23'h7fffff);
        2'h3:   ledcnten = (cnt25[21:0]==22'h3fffff);
        default ledcnten = 1'b0;
    endcase
end

/* LED用5進カウンタ */
reg [2:0] cnt3;

always @( posedge CLK ) begin
    if ( RST )
        cnt3 <= 3'h0;
    else if ( ledcnten )
        if ( cnt3==3'd4)
            cnt3 <=3'h0;
        else
            cnt3 <= cnt3 + 3'h1;
end

/* LEDデコーダ */
always @* begin
    case ( cnt3 )
        3'd0:   LED_RGB = ~3'b100;
        3'd1:   LED_RGB = ~3'b010;
        3'd2:   LED_RGB = ~3'b110;
        3'd3:   LED_RGB = ~3'b010;
        3'd4:   LED_RGB = ~3'b100;
        3'd5:   LED_RGB = ~3'b000;
        default:LED_RGB = ~3'b000;
    endcase
end

endmodule
