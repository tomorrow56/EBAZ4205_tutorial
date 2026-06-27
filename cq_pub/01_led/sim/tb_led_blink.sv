`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2026/06/28 07:39:25
// Design Name: 
// Module Name: tb_led_blink
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module tb_led_blink;

    //------------------------------------------------------------
    // Time Resolution
    //------------------------------------------------------------
    timeunit      1ns;
    timeprecision 1ps;

    //------------------------------------------------------------
    // DUT Signals
    //------------------------------------------------------------
    logic        CLK_0;
    logic [2:0]  Dout_0;

    //------------------------------------------------------------
    // DUT
    //------------------------------------------------------------
    led_blink_wrapper dut
    (
        .CLK_0 (CLK_0),
        .Dout_0(Dout_0)
    );

    //------------------------------------------------------------
    // Clock Generator
    //------------------------------------------------------------
    localparam time CLK_PERIOD = 30ns;

    initial begin
        CLK_0 = 0;
        forever #(CLK_PERIOD/2) CLK_0 = ~CLK_0;
    end

    //------------------------------------------------------------
    // Clocking Block
    //------------------------------------------------------------
    default clocking cb @(posedge CLK_0);
        input Dout_0;
    endclocking

    //------------------------------------------------------------
    // Wave Dump
    //------------------------------------------------------------
    initial begin
`ifdef DUMP_WAVE
        $dumpfile("tb_led_blink_wrapper.vcd");
        $dumpvars(0, tb_led_blink_wrapper);
`endif
    end

    //------------------------------------------------------------
    // Monitor
    //------------------------------------------------------------
    initial begin
        $display("-------------------------------------------");
        $display(" Time(ns)    Dout");
        $display("-------------------------------------------");

        forever begin
            @cb;
            $display("%8t      %03b", $time, cb.Dout_0);
        end
    end

    //------------------------------------------------------------
    // Assertions
    //------------------------------------------------------------

    //
    // 1. 出力がX/Zにならない
    //
    property p_no_unknown;
        !$isunknown(cb.Dout_0);
    endproperty

    assert property (p_no_unknown)
    else
        $fatal(1,
        "[ASSERT] Dout_0 contains X/Z at %0t", $time);

    //
    // 2. クロックが動いている
    //
    time last_clk;

    initial
        last_clk = 0;

    always @(posedge CLK_0) begin

        if(last_clk != 0) begin

            assert (($time-last_clk)==CLK_PERIOD)
            else
                $error("[ASSERT] Clock period error : %0t",
                        $time-last_clk);

        end

        last_clk = $time;

    end

    //
    // 3. 出力が変化したらカバー
    //
    property p_output_toggle;
        $changed(cb.Dout_0);
    endproperty

    cover property (p_output_toggle);

    //
    // 4. 全ビットが一度でも1になる
    //
    cover property (cb.Dout_0[0]);
    cover property (cb.Dout_0[1]);
    cover property (cb.Dout_0[2]);

    //------------------------------------------------------------
    // Simulation
    //------------------------------------------------------------
    initial begin

        $display("");
        $display("==============================");
        $display(" Simulation Start");
        $display("==============================");
        $display("");

        repeat (5_000_000)
            @cb;

        $display("");
        $display("==============================");
        $display(" Simulation Finished");
        $display("==============================");
        $display("");

        $finish;

    end

endmodule
