`timescale 1ns / 1ps
`include "defines.v"

module immediate_generator(
    input r,
    input [31:0] instruction,
    output reg [31:0] imm_sext
);

    always @(*) begin
        if (r) begin
            imm_sext = 0;
        end
        else begin
            case(instruction[6:0])
            `op_b : begin
                imm_sext = {{19{instruction[31]}}, instruction[31], instruction[7], instruction[30:25], instruction[11:8], 1'b0};
            end
            `op_i : begin
                imm_sext = {{20{instruction[31]}}, instruction[31:20]};
            end
            `op_s : begin
                imm_sext = {{20{instruction[31]}} ,instruction[31:25], instruction[11:7]};
            end
            `op_l : begin
                imm_sext = {{20{instruction[31]}}, instruction[31:20]};
            end
            `op_jal : begin
                imm_sext = {{12{instruction[31]}}, instruction[31], instruction[19:12], instruction[20], instruction[30:21]};
            end
            `op_jalr : begin
                imm_sext = {{20{instruction[31]}}, instruction[31:20]};
            end
            `op_lui : begin
                imm_sext = {{12{instruction[31]}}, instruction[31:12]};
            end
            `op_auipc : begin
                imm_sext = {{12{instruction[31]}}, instruction[31:12]};
            end
            default : begin
                imm_sext = 0;
            end
            endcase
        end
    end
    
endmodule