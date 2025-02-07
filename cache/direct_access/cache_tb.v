module cache_tb;
    
    reg clk;
    reg r;
    reg cpu2cache_rw;
    reg cpu2cache_valid;
    reg [31:0] cpu2cache_addr;
    reg [31:0] cpu2cache_data;
    wire cache2cpu_ready;
    wire cache2cpu_hit;
    wire [31:0] cache2cpu_data;
    wire cache2mem_rw;
    wire cache2mem_valid;
    wire [31:0] cache2mem_addr;
    wire [127:0] cache2mem_data;
    reg mem2cache_ready;
    reg [127:0] mem2cache_data;

    cache uut (
        .clk(clk),
        .r(r),
        .cpu2cache_rw(cpu2cache_rw),
        .cpu2cache_valid(cpu2cache_valid),
        .cpu2cache_addr(cpu2cache_addr),
        .cpu2cache_data(cpu2cache_data),
        .cache2cpu_ready(cache2cpu_ready),
        .cache2cpu_hit(cache2cpu_hit),
        .cache2cpu_data(cache2cpu_data),
        .cache2mem_rw(cache2mem_rw),
        .cache2mem_valid(cache2mem_valid),
        .cache2mem_addr(cache2mem_addr),
        .cache2mem_data(cache2mem_data),
        .mem2cache_ready(mem2cache_ready),
        .mem2cache_data(mem2cache_data)
    );

    initial begin
        clk = 0;
        r = 1;
        cpu2cache_rw = 0;
        cpu2cache_valid = 0;
        cpu2cache_addr = 0;
        cpu2cache_data = 0;
        mem2cache_ready = 0;
        mem2cache_data = 0;

        #10 r = 0; // Release reset

        // Test Read Miss
        #10 cpu2cache_addr = 32'h0000_1234;
        cpu2cache_valid = 1;
        cpu2cache_rw = 0;
        #10 cpu2cache_valid = 0;

        // Simulate memory response
        #20 mem2cache_ready = 1;
        mem2cache_data = 128'hDEADBEEF_DEADBEEF_DEADBEEF_DEADBEEF;
        #10 mem2cache_ready = 0;
        
        // Test Read Hit
        #10 cpu2cache_addr = 32'h0000_1234;
        cpu2cache_valid = 1;
        #10 cpu2cache_valid = 0;

        // Test Write Miss
        #10 cpu2cache_addr = 32'h0000_5678;
        cpu2cache_data = 32'hCAFEBABE;
        cpu2cache_rw = 1;
        cpu2cache_valid = 1;
        #10 cpu2cache_valid = 0;
        
        // Simulate memory response
        #20 mem2cache_ready = 1;
        mem2cache_data = 128'hFEEDFACE_FEEDFACE_FEEDFACE_FEEDFACE;
        #10 mem2cache_ready = 0;

        // Test Write Hit
        #10 cpu2cache_addr = 32'h0000_5678;
        cpu2cache_data = 32'hDEADBEEF;
        cpu2cache_rw = 1;
        cpu2cache_valid = 1;
        #10 cpu2cache_valid = 0;

        #100 $finish;
    end
    
    always #5 clk = ~clk;
endmodule
