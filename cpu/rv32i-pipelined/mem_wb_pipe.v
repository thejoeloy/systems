`timescale 1ns / 1ps

module MEMWB_pipe #(REGSIZE=32) (
    input clk,
    input r,
    input MEMWB_reg_write_i,
    input MEMWB_mem_2_reg_i,
    input [4:0] MEMWB_rd_i,
    input [REGSIZE-1:0] MEMWB_alu_result_i,
    //input [REGSIZE-1:0] MEMWB_read_data_i,
    output reg MEMWB_reg_write_o,
    output reg MEMWB_mem_2_reg_o,
    output reg [4:0] MEMWB_rd_o,
    output reg [REGSIZE-1:0] MEMWB_alu_result_o
    //output reg [REGSIZE-1:0] MEMWB_read_data_o
);
    
    always @(posedge clk) begin
        if (r) begin
            MEMWB_reg_write_o <= 0;
            MEMWB_mem_2_reg_o <= 0;
            MEMWB_rd_o <= 0;
            MEMWB_alu_result_o <= 0;
            //MEMWB_read_data_o <= 0;
        end
        else begin
            MEMWB_reg_write_o <= MEMWB_reg_write_i;
            MEMWB_mem_2_reg_o <= MEMWB_mem_2_reg_i;
            MEMWB_rd_o <= MEMWB_rd_i;
            MEMWB_alu_result_o <= MEMWB_alu_result_i;
            //MEMWB_read_data_o <= MEMWB_read_data_i;
        end
    end
    
endmodule