`timescale 1ns / 1ps
`include "defines.v"

// page 423 contains pseudocode

module hazard_unit #(ISTRSIZE=32) (
    input r,
    input IDEX_mem_read,
    input [4:0] IDEX_rd,
    input [4:0] IFID_rs1,
    input [4:0] IFID_rs2,
    output reg stall
);
    
    always @(*) begin
        if (r) begin
            stall = 1'b0;
        end
        else begin
            // Load use hazard RAW
            if (IDEX_mem_read &&
               ((IDEX_rd == IFID_rs1) || (IDEX_rd == IFID_rs2))) begin
                stall = 1'b1;   
            end
            else begin
                stall = 1'b0;
            end
            
        end
    end


endmodule