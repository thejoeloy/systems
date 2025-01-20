`include "defines.v"
`timescale 1ns / 1ps

// Page 351 of pdf (270 in actual textbook)

module alu (
	input r,
	input [6:0] op_code,
	input [9:0] func_code,
	input [31:0] alu_in1,
	input [31:0] alu_in2,
	output reg [31:0] alu_result
);
	
	wire signed [31:0] alu_in1_signed = alu_in1;
	wire signed [31:0] alu_in2_signed = alu_in2;
	
	
	always @(*) begin
		if (r) begin
            alu_result = 0;	
		end
		else begin
		  casex({func_code, op_code})
			{`f3_add, `f7_add, `op_add} : alu_result = alu_in1_signed + alu_in2_signed; // add
			{`f3_addi, 7'b???????, `op_addi} : alu_result = alu_in1_signed + alu_in2_signed; // addi
			{`f3_lb, 7'b???????, `op_lb} : alu_result = alu_in1_signed + alu_in2_signed;
			{`f3_lh, 7'b???????, `op_lh} : alu_result = alu_in1_signed + alu_in2_signed;
			{`f3_lw, 7'b???????, `op_lw} : alu_result = alu_in1_signed + alu_in2_signed;
			{`f3_lbu, 7'b???????, `op_lbu} : alu_result = alu_in1 + alu_in2;
			{`f3_lhu, 7'b???????, `op_lhu} : alu_result = alu_in1 + alu_in2;
			{`f3_sb, 7'b???????, `op_sb} : alu_result = alu_in1_signed[7:0] + alu_in2_signed[7:0];
			{`f3_sh, 7'b???????, `op_sh} : alu_result = alu_in1_signed[15:0] + alu_in2_signed[15:0];
			{`f3_sw, 7'b???????, `op_sw} : alu_result = alu_in1_signed + alu_in2_signed;
			{3'b???, 7'b???????, `op_jal} : alu_result = alu_in1 + alu_in2;
			
			{`f3_sub, `f7_sub, `op_sub} : alu_result = alu_in1_signed - alu_in2_signed; // sub
			
			{`f3_sll, `f7_sll, `op_sll} : alu_result = alu_in1_signed << alu_in2_signed; // sll
			{`f3_slli, `f7_slli, `op_slli} : alu_result = alu_in1_signed << alu_in2_signed; // slli
			
			{`f3_slt, `f7_slt, `op_slt} : alu_result = alu_in1_signed < alu_in2_signed; // slt
			{`f3_slti, 7'b???????, `op_slti} : alu_result = alu_in1_signed < alu_in2_signed;
			{`f3_sltu, `f7_sltu, `op_sltu} : alu_result = alu_in1 < alu_in2; // sltu
			{`f3_sltiu, 7'b???????, `op_sltiu} : alu_result = alu_in1 < alu_in2; // sltu
			
			{`f3_xor, `f7_xor, `op_xor} : alu_result = alu_in1_signed ^ alu_in2_signed; // xor
			{`f3_xori, 7'b???????, `op_xori} : alu_result = alu_in1_signed ^ alu_in2_signed; // xori
			
			{`f3_srl, `f7_srl, `op_srl} : alu_result = alu_in1_signed >> alu_in2_signed; // srl
			{`f3_srli, `f7_srli, `op_srli} : alu_result = alu_in1_signed >> alu_in2_signed; // srli
			{`f3_sra, `f7_sra, `op_sra} : alu_result = alu_in1_signed >>> alu_in2_signed; // sra
			{`f3_srai, `f7_srai, `op_srai} : alu_result = alu_in1_signed >>> alu_in2_signed; // sra
			
			{`f3_or, `f7_or, `op_or} : alu_result = alu_in1_signed | alu_in2_signed; // or
			{`f3_ori, 7'b???????, `op_ori} : alu_result = alu_in1_signed | alu_in2_signed; // ori
			
			{`f3_and, `f7_and, `op_and} : alu_result = alu_in1_signed & alu_in2_signed; // and 
			{`f3_andi, 7'b???????, `op_andi} : alu_result = alu_in1_signed & alu_in2_signed; // andi 
			
			{3'b???, 7'b???????, `op_lui} : alu_result = alu_in2_signed[31:12] << 12; // lui
			{3'b???, 7'b???????, `op_auipc} : alu_result = alu_in1_signed + (alu_in2_signed[31:12] << 12); // auipc
			
			default : alu_result = 0;
		endcase
		end
	end
	
endmodule
