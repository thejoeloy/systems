`timescale 1ns / 1ps
module instruction_memory (
	input r,
	input [9:0] read_addr,
	output reg [31:0] instr
	//input [31:0] i_mem_addr,
	//input [31:0] i_mem_data,
	//input i_mem_write
);

    localparam MEMSIZE = 65536;
    //localparam MEMSIZE = 1024;
    localparam MEMWIDTH = 32;
    reg [MEMWIDTH-1:0] i_memory[0:MEMSIZE-1];
	/*
	integer i;
	initial begin
	   for (i = 0; i < MEMSIZE; i = i + 1) begin
	      i_memory[i] = 0;
	   end
	end
	*/
	
	integer i;
	initial begin
	   for (i = 0; i < MEMSIZE; i = i + 1) begin
            
            if (i == 0) begin
                i_memory[i] = 32'b0000000_00010_00001_000_00011_0110011; // add x3, x1, x2        
            end
            
            else if (i == 1) begin
                i_memory[i] = 32'b0100000_00001_00010_000_00100_0110011; // sub x4, x1, x2
            end
            
            else if (i == 2) begin
                i_memory[i] = 32'b0000000_00010_00001_111_00101_0110011; // and x5, x1, x3
            end
            
            else if(i == 3) begin
                i_memory[i] = 32'b0000000_00010_00100_110_00110_0110011; // or x6, x4, x2           
            end
            
            else if (i == 4) begin
                i_memory[i] = 32'b000000000100_00000_010_00111_0000011; // lw x7, 4(x0)          
            end
            
            else if (i == 5) begin
                i_memory[i] = 32'b0000000_00111_00000_010_00000_0100011; // sw x7, 0(x0)
            end
            
            else if (i == 6) begin
                i_memory[i] = 32'b0000000_00110_00010_000_01010_1100011; // beq x2, x6, offset
            end
            
            else if (i == 16) begin
                i_memory[i] = 32'b0000000_00011_00001_001_01000_0110011; // sll x8, x3, x1
            end
            
            else if (i == 17) begin
                i_memory[i] = 32'b0000000_00011_00001_010_01001_0110011; // slt x9, x3, x1
            end
            
            else if (i == 18) begin
                i_memory[i] = 32'b0000000_00011_00001_100_01010_0110011; // xor x10, x3, x1
            end
            
            else if (i == 19) begin
                i_memory[i] = 32'b0000000_00001_00011_101_01011_0110011; // srl x11, x3, x1
            end
            
            else if (i == 20) begin
                i_memory[i] = 32'b0100000_00001_00011_101_01100_0110011; // sra x12, x3, x1
            end
            
            else if (i == 21) begin
                i_memory[i] = 32'b000000000101_01100_000_01101_0010011; // addi x13, x12, 5
            end
            
            else if (i == 22) begin
                i_memory[i] = 32'b000000000001_01101_010_01110_0010011; // slti x14, x13, 1
            end
            
            else if (i == 23) begin
                i_memory[i] = 32'b000000000101_01101_110_01111_0010011; // ori x15, x14, 5
            end
            
            else if (i == 24) begin
                i_memory[i] = 32'b000000000001_01111_111_10000_0010011; // andi x16, x15, 1
            end
            
            else if (i == 25) begin
                i_memory[i] = 32'b00000001000000000000_10001_1101111; // jal x17, 16
            end
            
            else if (i == 34) begin
                i_memory[i] = 32'b000000000100_00001_000_10010_1100111; // jalr x18, 4(x1)
            end
            
            else if (i == 35) begin
                i_memory[i] = 32'b0000000000000000100_10011_0110111; // lui x19, 4
            end
            
            else if (i == 36) begin
                i_memory[i] = 32'b0000000000000001000_10100_0010111; // auipc x20, 8
            end
            
            else begin
                i_memory[i] = 0;
            end	   
	    end
	end
	
	always @(*) begin
		if (r) begin
			instr = 32'b0;
		end
		/*
		else if (i_mem_write) begin
		    i_memory[i_mem_addr[9:0]] = i_mem_data;
		    instr = instr;
		end
		*/
		else begin
			instr = i_memory[read_addr[9:0]];
		end
	end
    
endmodule

/*
Byte addressable
`timescale 1ns / 1ps
module instruction_memory (
    input r,
    input [9:0] read_addr,
    output reg [7:0] instr_byte,
    input [7:0] i_mem_data_byte,
    input [9:0] i_mem_addr_byte,
    input i_mem_write
);

    localparam MEMSIZE = 65536;
    localparam MEMWIDTH = 32;
    reg [MEMWIDTH-1:0] i_memory[0:MEMSIZE-1];

    integer i;
    initial begin
        // Initialize memory with zeros
        for (i = 0; i < MEMSIZE; i = i + 1) begin
            i_memory[i] = 32'b0;
        end
        
        // Initialize specific addresses with your instructions
        i_memory[0] = 32'b0000000_00010_00001_000_00011_0110011; // add x3, x1, x2
        // ... (initialize other instructions as needed)
    end

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            // Reset logic if needed
            instr_byte <= 8'b0;
        end else if (r) begin
            instr_byte <= i_memory[read_addr[9:0]][7:0];
        end else if (i_mem_write) begin
            i_memory[i_mem_addr_byte[9:0]] <= {i_memory[i_mem_addr_byte[9:0]][31:8], i_mem_data_byte};
        end
    end

endmodule

*/

