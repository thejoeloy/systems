`timescale 1ns / 1ps

module cache_tb();
    reg clk;
    reg r;
    reg cpu2cache_valid;
    reg cpu2cache_rw;
    reg [31:0] cpu2cache_addr;
    reg [31:0] cpu2cache_data;
    wire [31:0] cache2cpu_data;
    wire cache2cpu_ready;
    wire cache2mem_valid;
    wire cache2mem_rw;
    wire [31:0] cache2mem_addr;
    wire [127:0] cache2mem_data;
    reg [127:0] mem2cache_data;
    reg mem2cache_ready;

    cache uut(
        .clk(clk),
        .r(r),
        .cpu2cache_valid(cpu2cache_valid),
        .cpu2cache_rw(cpu2cache_rw),
        .cpu2cache_addr(cpu2cache_addr),
        .cpu2cache_data(cpu2cache_data),
        .cache2cpu_data(cache2cpu_data),
        .cache2cpu_ready(cache2cpu_ready),
        .cache2mem_valid(cache2mem_valid),
        .cache2mem_rw(cache2mem_rw),
        .cache2mem_addr(cache2mem_addr),
        .cache2mem_data(cache2mem_data),
        .mem2cache_data(mem2cache_data),
        .mem2cache_ready(mem2cache_ready)
    );


    always #5 clk = ~clk;

    initial begin
        $dumpfile("cache_tb.vcd");
        $dumpvars(0, cache_tb);

        clk = 1;
        r = 1;
        cpu2cache_valid = 0;
        cpu2cache_rw = 0;
        cpu2cache_addr = 0;
        cpu2cache_data = 0;
        mem2cache_data = 0;
        mem2cache_ready = 0;

        #10 r = 0;

        // Read Miss (valid tag not set)
        cpu2cache_valid = 1;
        cpu2cache_addr = 32'b000000000000000000_0000000000_00_00;
        #20;
        mem2cache_ready = 1;
        cpu2cache_valid = 0;
        #10;
        mem2cache_ready = 0;

        // Read Hit
        cpu2cache_valid = 1;
        cpu2cache_addr = 32'b000000000000000001_0000000100_00_00;
        #20;
        mem2cache_ready = 1;
        cpu2cache_valid = 0;
        #10;
        mem2cache_ready = 0;
        
        // TO DO EDIT WRITES!!!
        // Write Miss
        cpu2cache_valid = 1;
        cpu2cache_addr = 32'b000000000000000010_0000001000_00_00;
        #20;
        mem2cache_ready = 1;
        cpu2cache_valid = 0;
        cpu2cache_rw = 1;
        #10;
        //mem2cache_ready = 0;
        //#15;
        //mem2cache_ready = 1;
        #10;
        mem2cache_ready = 0;
        // Write Hit
        cpu2cache_valid = 1;
        cpu2cache_addr = 32'b000000000000000011_0000001100_00_00;
        cpu2cache_data = 32'hFFFFFFFF;
        #20;
        cpu2cache_valid = 0;

        #20 $finish;
    end
endmodule
