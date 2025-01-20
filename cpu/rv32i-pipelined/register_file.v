`timescale 1ns / 1ps
module register_file (
    input clk,
    input wire r,
    input wire [4:0] rs1, 
    input wire [4:0] rs2, 
    input wire [4:0] write_addr,
    input wire write_enable,
    input wire [31:0] write_data,
    output reg [31:0] read_data1,
    output reg [31:0] read_data2
);

    localparam REGSIZE = 32;
    localparam NUMREGS = 32;
    reg [REGSIZE-1:0] regs[0:NUMREGS - 1];
    /*
    integer i;
    initial begin
       for (i = 0; i < NUMREGS; i = i + 1) begin
           regs[i] = 0;
       end
    end
    */
    integer i;
    initial begin
       for (i = 0; i < NUMREGS; i = i + 1) begin
           if (i == 1) begin
               regs[i] = 1;
           end
           
           else if (i == 2) begin
               regs[i] = 5;
           end
           
           else begin
               regs[i] = 0;
           end
       end
    end
    
    // Write Operation (positive edge of clk)
    always @(posedge clk) begin
        if (write_enable) begin
            regs[write_addr] <= write_data;
        end
    end

    // Read Operation (negative edge of clk)
    always @(negedge clk) begin
        if (!r) begin
            read_data1 <= (rs1 == 0) ? 32'd0 : regs[rs1];
            read_data2 <= (rs2 == 0) ? 32'd0 : regs[rs2];
        end
    end
endmodule