`timescale 1ns / 1ps

module IDEX_pipe #(PCSIZE=32, REGSIZE=32) (
    input clk,
    input r,
    input [4:0] IDEX_rs1_i,
    input [4:0] IDEX_rs2_i,
    input [4:0] IDEX_rd_i,
    input [PCSIZE-1:0] IDEX_imm_sext_i,
    input [REGSIZE-1:0] IDEX_read_data1_i,
    input [REGSIZE-1:0] IDEX_read_data2_i,
    input IDEX_reg_write_i,
    input IDEX_mem_2_reg_i,
    input IDEX_mem_read_i,
    input IDEX_mem_write_i,
    input IDEX_alu_src_i,
    input [6:0] IDEX_op_code_i,
    input [9:0] IDEX_func_code_i,
    output reg [4:0] IDEX_rs1_o,
    output reg [4:0] IDEX_rs2_o,
    output reg [4:0] IDEX_rd_o,
    output reg [PCSIZE-1:0] IDEX_imm_sext_o,
    output reg [REGSIZE-1:0] IDEX_read_data1_o,
    output reg [REGSIZE-1:0] IDEX_read_data2_o,
    output reg IDEX_reg_write_o,
    output reg IDEX_mem_2_reg_o,
    output reg IDEX_mem_read_o,
    output reg IDEX_mem_write_o,
    output reg IDEX_alu_src_o,
    output reg [6:0] IDEX_op_code_o,
    output reg [9:0] IDEX_func_code_o
);
   
    always @(posedge clk) begin
        if (r) begin
            IDEX_rs1_o <= 0; 
            IDEX_rs2_o <= 0; 
            IDEX_rd_o <= 0;  
            IDEX_imm_sext_o <= 0; 
            IDEX_read_data1_o <= 0; 
            IDEX_read_data2_o <= 0; 
            IDEX_reg_write_o <= 0; 
            IDEX_mem_2_reg_o <= 0; 
            IDEX_mem_read_o <= 0; 
            IDEX_mem_write_o <= 0; 
            IDEX_alu_src_o <= 0;    
            IDEX_op_code_o <= 0;
            IDEX_func_code_o <= 0;
        end
        else begin
            IDEX_rs1_o <= IDEX_rs1_i; 
            IDEX_rs2_o <= IDEX_rs2_i; 
            IDEX_rd_o <= IDEX_rd_i;
            IDEX_imm_sext_o <= IDEX_imm_sext_i; 
            IDEX_read_data1_o <= IDEX_read_data1_i; 
            IDEX_read_data2_o <= IDEX_read_data2_i; 
            IDEX_reg_write_o <= IDEX_reg_write_i; 
            IDEX_mem_2_reg_o <= IDEX_mem_2_reg_i; 
            IDEX_mem_read_o <= IDEX_mem_read_i; 
            IDEX_mem_write_o <= IDEX_mem_write_i; 
            IDEX_alu_src_o <= IDEX_alu_src_i;
            IDEX_op_code_o <= IDEX_op_code_i;
            IDEX_func_code_o <= IDEX_func_code_i;
        end
    end

endmodule