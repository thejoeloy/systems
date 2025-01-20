`timescale 1ns / 1ps

// page 421 contains pseudocode

module forward_unit (
    input r,
    input EXMEM_reg_write,
    input MEMWB_reg_write,
    input [4:0] IDEX_rs1,
    input [4:0] IDEX_rs2,
    input [4:0] EXMEM_rd,
    input [4:0] MEMWB_rd,
    output reg [1:0] forwardA,
    output reg [1:0] forwardB
);

    always @(*) begin
        if (r) begin
            forwardA = 0;
            forwardB = 0;
        end
        else begin
            if (EXMEM_reg_write &&
            (EXMEM_rd != 0) &&
            (EXMEM_rd == IDEX_rs1)) begin
           
                forwardA = 2'b10;
            end
            else if (MEMWB_reg_write && 
                    (MEMWB_rd != 0) && 
                    ((MEMWB_rd != EXMEM_rd) || !EXMEM_reg_write) &&
                    (MEMWB_rd == IDEX_rs1)) begin
            
            forwardA = 2'b01;   
            end
            else begin
                forwardA = 2'b00;
            end 
                
            if  (EXMEM_reg_write &&
                (EXMEM_rd != 0) &&
                (EXMEM_rd == IDEX_rs2)) begin
           
                forwardB = 2'b10;
            end         
            else if (MEMWB_reg_write && 
                    (MEMWB_rd != 0) && 
                    ((MEMWB_rd != EXMEM_rd) || !EXMEM_reg_write) &&
                    (MEMWB_rd == IDEX_rs2)) begin
            
                    forwardB = 2'b01;   
            end
            else begin
                forwardB = 2'b00;            
            end                       
        end
    end
endmodule