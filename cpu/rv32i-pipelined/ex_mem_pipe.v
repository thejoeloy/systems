`timescale 1ns / 1ps

module EXMEM_pipe #(REGSIZE=32) (
    input clk,
    input r,
    input [REGSIZE-1:0] EXMEM_alu_result_i,
    input [REGSIZE-1:0] EXMEM_alu_in2_i,
    input [4:0] EXMEM_rd_i,
    input EXMEM_reg_write_i,
    input EXMEM_mem_2_reg_i,
    input EXMEM_mem_read_i,
    input EXMEM_mem_write_i,
    output reg [REGSIZE-1:0] EXMEM_alu_result_o,
    output reg [REGSIZE-1:0] EXMEM_alu_in2_o,
    output reg [4:0] EXMEM_rd_o,
    output reg EXMEM_reg_write_o,
    output reg EXMEM_mem_2_reg_o,
    output reg EXMEM_mem_read_o,
    output reg EXMEM_mem_write_o
);

    always @(posedge clk) begin
        if (r) begin
            EXMEM_alu_result_o <= 0;
            EXMEM_alu_in2_o <= 0;
            EXMEM_rd_o <= 0;
            EXMEM_reg_write_o <= 0;
            EXMEM_mem_2_reg_o <= 0;
            EXMEM_mem_read_o <= 0;
            EXMEM_mem_write_o <= 0;
        end
        else begin
            EXMEM_alu_result_o <= EXMEM_alu_result_i;
            EXMEM_alu_in2_o <= EXMEM_alu_in2_i;
            EXMEM_rd_o <= EXMEM_rd_i;
            EXMEM_reg_write_o <= EXMEM_reg_write_i;
            EXMEM_mem_2_reg_o <= EXMEM_mem_2_reg_i;
            EXMEM_mem_read_o <= EXMEM_mem_read_i;
            EXMEM_mem_write_o <= EXMEM_mem_write_i;
        end
    end
endmodule