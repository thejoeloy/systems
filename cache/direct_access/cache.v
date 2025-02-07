
module cache (
    input clk,
    input r,
    // Cache <-> Processor Signals
    input cpu2cache_rw,                                       // Request type: read = 0, write = 1
    input cpu2cache_valid,                                    // Request is valid
    input wire [ADDR_WIDTH-1:0] cpu2cache_addr,
    input wire [DATA_WIDTH-1:0] cpu2cache_data,
    output wire cache2cpu_ready,
    output wire cache2cpu_hit,
    output wire [DATA_WIDTH-1:0] cache2cpu_data,   
    // Cache <-> Memory Signals
    output wire cache2mem_rw,
    output wire cache2mem_valid,
    output wire [ADDR_WIDTH-1:0] cache2mem_addr,
    output wire [DATA_BLOCK_SIZE-1:0] cache2mem_data,
    input wire mem2cache_ready,
    output wire [DATA_BLOCK_SIZE-1:0] mem2cache_data
);

    // Parameters for address and data entries
    localparam INDEX_WIDTH = 10;
    localparam BLOCK_OFFSET = 2;
    localparam BYTE_OFFSET = 2;
    localparam ADDR_WIDTH = 32;
    localparam DATA_WIDTH = 32;
    localparam TAG_MSB = 31;
    localparam TAG_LSB = 14;
    localparam TAG_LEN = 18;
    localparam INDEX_MSB = 13;
    localparam INDEX_LSB = 4;
    localparam INDEX_LEN = 10;
    localparam BLOCK_OFFSET_MSB = 3;
    localparam BLOCK_OFFSET_LSB = 2;
    localparam BLOCK_OFFSET_LEN = 2;
    localparam BYTE_OFFSET_MSB = 1;
    localparam BYTE_OFFSET_LSB = 0;
    localparam BYTE_OFFSET_LEN = 2;
    
    // cpu req address fields
    wire [TAG_LEN-1:0] req_tag = cpu2cache_addr[TAG_MSB:TAG_LSB];
    wire [INDEX_LEN-1:0] req_index = cpu2cache_addr[INDEX_MSB:INDEX_LSB];
    wire [BLOCK_OFFSET_LEN-1:0] req_block_offset = cpu2cache_addr[BLOCK_OFFSET_MSB:BLOCK_OFFSET_LSB];
    wire [BYTE_OFFSET_LEN-1:0] req_byte_offset = cpu2cache_addr[BYTE_OFFSET_MSB:BYTE_OFFSET_LSB];

    // Parameters for data cache 
    localparam DATA_BLOCK_SIZE = 128;
    localparam NUM_DATA_ENTRIES = 1024;
    localparam BLOCK1_MSB = 31;
    localparam BLOCK1_LSB = 0;
    localparam BLOCK2_MSB = 63;
    localparam BLOCK2_LSB = 32;
    localparam BLOCK3_MSB = 95;
    localparam BLOCK3_LSB = 64;
    localparam BLOCK4_MSB = 127;
    localparam BLOCK4_LSB = 96;
    
    reg [DATA_BLOCK_SIZE-1:0] cache_data [0:NUM_DATA_ENTRIES-1];
    wire [DATA_BLOCK_SIZE-1:0] cache_block = cache_data[req_index];

    // Initialize cache to 0 values
    initial begin
        integer i;
        for (i = 0; i < NUM_DATA_ENTRIES; i = i + 1)
            cache_data[i] = 0;
    end

    // Parameters for tag cache 
    localparam TAG_BLOCK_SIZE = 20; 
    localparam VALID_BIT = 19;
    localparam DIRTY_BIT = 18;
    
    reg [TAG_BLOCK_SIZE-1:0] tag_data [0:NUM_DATA_ENTRIES-1];
  
    wire is_valid = tag_data[req_index][VALID_BIT];
    wire is_dirty = tag_data[req_index][DIRTY_BIT];
    wire [TAG_WIDTH-1:0] tag = tag_data[req_index][TAG_WIDTH-1:0];
   
    // Initialize tags to 0 values
    initial begin
        integer j;
        for (j = 0; j < NUM_DATA_ENTRIES; j = j + 1)
            tag_data[j] = 0;
    end

    // States for cache controller
    localparam IDLE = 2'b00;
    localparam COMPARE_TAG = 2'b01;
    localparam ALLOCATE = 2'b10;
    localparam WRITE_BACK = 2'b11;

    // FSM for cache controller
    reg [1:0] state;

    wire [DATA_BLOCK_SIZE-1:0] data_write;
    wire [DATA_BLOCK_SIZE-1:0] data_read;

    
    always @(*) begin
            
    end

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
                   state <= (cache2cpu_hit) ? IDLE :
                            (!cache2cpu_hit && !is_dirty) ? ALLOCATE :
                            (!cache2cpu_hit && is_dirty) ? WRITE_BACK : 
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


    assign cache2cpu_hit = (req_tag == tag && is_valid) ? 1'b1 : 1'b0;
    assign cache2cpu_data = (cpu_req_block_offset == 2'b00) ? cache_data_read[BLOCK1_MSB:BLOCK1_LSB] :
                        (cpu_req_block_offset == 2'b01) ? cache_data_read[BLOCK2_MSB:BLOCK2_LSB] :
                        (cpu_req_block_offset == 2'b10) ? cache_data_read[BLOCK3_MSB:BLOCK3_LSB] :
                        cache_data_read[BLOCK4_MSB:BLOCK4_LSB];

endmodule
