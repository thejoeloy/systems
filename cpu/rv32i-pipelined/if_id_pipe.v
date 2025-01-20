`timescale 1ns / 1ps

module IFID_pipe #(PCSIZE=32, ISTRSIZE=32) (
    input clk,
    input r,
    input stall,
    input if_flush,
    input [ISTRSIZE-1:0] IFID_instr_i,
    input [PCSIZE-1:0] IFID_pc_i,
    output reg [ISTRSIZE-1:0] IFID_instr_o,
    output reg [PCSIZE-1:0] IFID_pc_o
    );
    
    always @(posedge clk) begin
        if (r) begin
            IFID_pc_o <= 0;
            IFID_instr_o <= 0;
        end
        else if (if_flush) begin
            IFID_instr_o <= 0;
        end
        else if (stall) begin
            IFID_pc_o <= IFID_pc_o;
            IFID_instr_o <= IFID_instr_o;
        end
        else begin
            IFID_pc_o <= IFID_pc_i;
            IFID_instr_o <= IFID_instr_i;
        end
    end
    
endmodule