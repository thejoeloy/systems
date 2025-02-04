
module cache (
    input clk,
    input r,
    input rw,                                       // Request type: read = 0, write = 1
    input valid,                                    // Request is valid
    input wire [ADDR_WIDTH-1:0] cpu_req_addr,
    input wire [DATA_WIDTH-1:0] cpu_req_data,
    input wire mem_ready,
    input wire [DATA_WIDTH-1:0] mem_data,
    output wire [DATA_WIDTH-1:0] cache_data,
    output wire cache_hit
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
    wire [TAG_LEN-1:0] cpu_req_tag = cpu_req_addr[TAG_MSB:TAG_LSB];
    wire [INDEX_LEN-1:0] cpu_req_index = cpu_req_addr[INDEX_MSB:INDEX_LSB];
    wire [BLOCK_OFFSET_LEN-1:0] cpu_req_block_offset = cpu_req_addr[BLOCK_OFFSET_MSB:BLOCK_OFFSET_LSB];
    wire [BYTE_OFFSET_LEN-1:0] cpu_req_byte_offset = cpu_req_addr[BYTE_OFFSET_MSB:BYTE_OFFSET_LSB];

    // Parameters for data cache 
    localparam DATA_BLOCK_SIZE = 128;
    localparam NUM_DATA_ENTRIES = 1024;
    localparam BLOCK1_MSB = 127;
    localparam BLOCK1_LSB = 96;
    localparam BLOCK2_MSB = 95;
    localparam BLOCK2_LSB = 64;
    localparam BLOCK3_MSB = 63;
    localparam BLOCK3_LSB = 32;
    localparam BLOCK4_MSB = 31;
    localparam BLOCK4_LSB = 0;
    
    reg [DATA_BLOCK_SIZE-1:0] cache_data [0:NUM_DATA_ENTRIES-1];
    wire [DATA_BLOCK_SIZE-1:0] cache_block = cache_data[DATA_BLOCK_SIZE-1:0][cpu_req_index];

    // Parameters for tag cache 
    localparam TAG_BLOCK_SIZE = 20; 
    localparam TAG_WIDTH = 18;
    localparam VALID_BIT = 19;
    localparam DIRTY_BIT = 18;
    
    reg [TAG_BLOCK_SIZE-1:0] tag_data [0:NUM_DATA_ENTRIES-1];
  
    wire is_valid = tag_data[TAG_BIT][cpu_req_index];
    wire is_dirty = tag_data[DIRTY_BIT][cpu_req_index];
    wire [INDEX_LEN-1:0] tag = tag_data[INDEX_LEN-1:0][cpu_req_index];
   
    // States for cache controller
    localparam IDLE = 2'b00;
    localparam COMPARE_TAG = 2'b01;
    localparam ALLOCATE = 2'b10;
    localparam WRITE_BACK = 2'b11;

    // FSM for cache controller
    reg [1:0] state;

    always @(posedge clk or posedge r) begin
        if (r) begin
            state <= IDLE;
        end
        else begin
            case (state)
                IDLE : begin
                    state = (valid) ? COMPARE_TAG : IDLE;
                end
                COMPARE_TAG : begin
                    state = (!hit && is_dirty) ? WRITE_BACK :
                            (!hit && !is_dirty) ? ALLOCATE :
                            (hit) ? IDLE : COMPARE_TAG;
                end
                ALLOCATE : begin
                    state = (mem_ready) ? COMPARE_TAG : ALLOCATE;
                end
                WRITE_BACK : begin
                    state = (mem_ready) ? ALLOCATE : WRITE_BACK;
                end
            endcase
        end
    end

    wire [DATA_BLOCK_SIZE-1:0] cache_data_write;
    wire [DATA_BLOCK_SIZE-1:0] cache_data_read;

    assign cache_data_read = cache_data[cpu_req_index];

    assign cache_data_write = () ? :
                              () ? :
                              () ? :


    always @(posedge clk or posedge r) begin
        integer i;
        if (r) begin
            for (i = 0; i < NUM_DATA_ENTRIES - 1; i = i + 1) begin
                cache_data[i] <= 128'b0;
            end
        end
        else begin
            if (valid) begin
                // If rw is 1, we write
                if (rw) begin
                    cache_data[DATA_BLOCK_SIZE-1:0][cpu_req_index] <= cache_data_write;
                end
                // Otherwise we read
                else begin
                    cache_data_read <= cache_data[DATA_BLOCK_SIZE-1:0][cpu_req_index];
                end
            end
        end
    end
    
    assign cache_hit = (cpu_req_tag == tag && is_valid) ? 1'b1 : 1'b0;
    assign cache_data = (cpu_req_block_offset == 2'b00) ? cache_data_read[BLOCK1_MSB:BLOCK1_LSB] :
                        (cpu_req_block_offset == 2'b01) ? cache_data_read[BLOCK2_MSB:BLOCK2_LSB] :
                        (cpu_req_block_offset == 2'b10) ? cache_data_read[BLOCK3_MSB:BLOCK3_LSB] :
                        cache_data_read[BLOCK4_MSB:BLOCK4_LSB];

endmodule
