`timescale 1ns / 1ps

module direct_mapped_cache (
    input wire clk,
    input wire rst,
    input wire cpu2cache_valid,
    input wire cpu2cache_rw, // 0 for read, 1 for write
    input wire [31:0] cpu2cache_addr,
    input wire [31:0] cpu2cache_data,
    output reg cache2cpu_ready,
    output reg [31:0] cache2cpu_data,
    output reg cache2cpu_hit,
    
    // Memory interface
    output reg cache2mem_valid,
    output reg cache2mem_rw,
    output reg [31:0] cache2mem_addr,
    output reg [127:0] cache2mem_data,
    input wire [127:0] mem2cache_data,
    input wire mem2cache_ready
);

    // Cache parameters
    parameter INDEX_LEN = 8;
    parameter TAG_LEN = 22;
    parameter BLOCK_OFFSET_LEN = 2;
    parameter CACHE_SIZE = 256;
    
    // Cache storage
    reg [127:0] cache_data[CACHE_SIZE-1:0];
    reg [TAG_LEN:0] tag_data[CACHE_SIZE-1:0]; // [VALID (1 bit), DIRTY (1 bit), TAG (TAG_LEN bits)]
    
    // Address breakdown
    wire [TAG_LEN-1:0] req_tag = cpu2cache_addr[31:INDEX_LEN+BLOCK_OFFSET_LEN];
    wire [INDEX_LEN-1:0] req_index = cpu2cache_addr[INDEX_LEN+BLOCK_OFFSET_LEN-1:BLOCK_OFFSET_LEN];
    wire [BLOCK_OFFSET_LEN-1:0] req_block_offset = cpu2cache_addr[BLOCK_OFFSET_LEN-1:0];
    
    // Extract tag, valid, and dirty bits
    wire [TAG_LEN-1:0] stored_tag = tag_data[req_index][TAG_LEN-1:0];
    wire is_valid = tag_data[req_index][TAG_LEN];
    wire is_dirty = tag_data[req_index][TAG_LEN+1];
    
    // FSM states
    typedef enum reg [1:0] {IDLE, COMPARE_TAG, ALLOCATE, WRITE_BACK} state_t;
    state_t state;
    
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            state <= IDLE;
            cache2cpu_ready <= 0;
            cache2cpu_hit <= 0;
            cache2mem_valid <= 0;
        end else begin
            case (state)
                IDLE: begin
                    if (cpu2cache_valid) begin
                        state <= COMPARE_TAG;
                    end
                end
                
                COMPARE_TAG: begin
                    if (is_valid && (stored_tag == req_tag)) begin // Cache hit
                        cache2cpu_hit <= 1'b1;
                        cache2cpu_ready <= 1'b1;
                        case (req_block_offset)
                            2'b00: cache2cpu_data <= cache_data[req_index][31:0];
                            2'b01: cache2cpu_data <= cache_data[req_index][63:32];
                            2'b10: cache2cpu_data <= cache_data[req_index][95:64];
                            2'b11: cache2cpu_data <= cache_data[req_index][127:96];
                        endcase
                        if (cpu2cache_rw) begin // Write request
                            case (req_block_offset)
                                2'b00: cache_data[req_index][31:0] <= cpu2cache_data;
                                2'b01: cache_data[req_index][63:32] <= cpu2cache_data;
                                2'b10: cache_data[req_index][95:64] <= cpu2cache_data;
                                2'b11: cache_data[req_index][127:96] <= cpu2cache_data;
                            endcase
                            tag_data[req_index][TAG_LEN+1] <= 1'b1; // Mark as dirty
                        end
                        state <= IDLE;
                    end else begin // Cache miss
                        cache2cpu_hit <= 1'b0;
                        if (is_dirty) begin
                            state <= WRITE_BACK;
                        end else begin
                            state <= ALLOCATE;
                        end
                    end
                end
                
                WRITE_BACK: begin
                    cache2mem_valid <= 1'b1;
                    cache2mem_rw <= 1'b1;
                    cache2mem_addr <= {stored_tag, req_index, {BLOCK_OFFSET_LEN{1'b0}}};
                    cache2mem_data <= cache_data[req_index];
                    
                    if (mem2cache_ready) begin
                        cache2mem_valid <= 0;
                        state <= ALLOCATE;
                    end
                end
                
                ALLOCATE: begin
                    cache2mem_valid <= 1'b1;
                    cache2mem_rw <= 1'b0;
                    cache2mem_addr <= {req_tag, req_index, {BLOCK_OFFSET_LEN{1'b0}}};
                    
                    if (mem2cache_ready) begin
                        cache2mem_valid <= 0;
                        cache_data[req_index] <= mem2cache_data;
                        tag_data[req_index] <= {1'b1, 1'b0, req_tag}; // Valid, not dirty, new tag
                        state <= COMPARE_TAG;
                    end
                end
            endcase
        end
    end
endmodule
