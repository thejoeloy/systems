`timescale 1ns / 1ps

module cache (
    input wire clk,
    input wire r,
    // CPU <-> Cache
    input wire cpu2cache_valid,
    input wire cpu2cache_rw,
    input wire [31:0] cpu2cache_addr,
    input wire [31:0] cpu2cache_data,
    output wire [31:0] cache2cpu_data,
    output wire cache2cpu_ready,
    // Cache <-> Memory
    output cache2mem_valid,
    output cache2mem_rw,
    output [31:0] cache2mem_addr,
    output [127:0] cache2mem_data,
    input [127:0] mem2cache_data,
    input mem2cache_ready
);

    localparam TAG_LEN = 18;
    localparam INDEX_LEN = 10;
    localparam BLOCK_OFFSET_LEN = 2;
    localparam BYTE_OFFSET_LEN = 2;
    localparam CACHE_SIZE = 1024;

    reg [127:0] cache_data[0:CACHE_SIZE-1];
    reg [19:0] tag_data[0:CACHE_SIZE-1];
    
    integer i;
    initial begin
        for (i = 0; i < CACHE_SIZE; i = i + 1) begin
            if (i == 0) begin
                tag_data[i] = 20'b0_0_000000000000000000;
                cache_data[i] = 32'h0000000F;
            end
            else if (i == 4) begin
                tag_data[i] = 20'b1_0_000000000000000001;
                cache_data[i] = 32'h000000F0;
            end
            else if (i == 8) begin
                tag_data[i] = 20'b0_1_000000000000000010;
                cache_data[i] = 32'h00000F00;
            end
            else if (i == 12) begin
                tag_data[i] = 20'b0_0_000000000000000011;
                cache_data[i] = 32'h0000F000;
            end
            else begin
                cache_data[i] = 0;
                tag_data[i] = 0;
            end
        end
    end
    
    wire [TAG_LEN-1:0] req_tag = cpu2cache_addr[31:14];
    wire [INDEX_LEN-1:0] req_index = cpu2cache_addr[13:4];
    wire [BLOCK_OFFSET_LEN-1:0] req_block_offset = cpu2cache_addr[3:2];

    wire [TAG_LEN-1:0] tag = tag_data[req_index][TAG_LEN-1:0];
    wire is_dirty = tag_data[req_index][18];
    wire is_valid = tag_data[req_index][19];
    wire hit = (req_tag == tag && is_valid) ? 1'b1 : 1'b0;

    localparam IDLE = 2'b00;
    localparam COMPARE_TAG = 2'b01;
    localparam ALLOCATE = 2'b10;
    localparam WRITE_BACK = 2'b11;
    reg [1:0] state;

    always @(posedge clk) begin
        if (r) begin
            state <= IDLE;

        end
        else begin
            case(state)
                IDLE : begin
                    state <= (cpu2cache_valid) ? COMPARE_TAG : IDLE; 
                end
                COMPARE_TAG : begin
                    state <= (hit)               ? IDLE :
                             (!hit && !is_dirty) ? ALLOCATE :
                             (!hit && is_dirty)  ? WRITE_BACK : 
                                                   COMPARE_TAG;


                end
                ALLOCATE : begin
                    state <= (mem2cache_ready) ? COMPARE_TAG : ALLOCATE;    
                end
                WRITE_BACK : begin
                    state <= (mem2cache_ready) ? ALLOCATE : WRITE_BACK;
                end
            endcase
        end
    end

endmodule
