iverilog -o cache_sim cache_tb.v cache.v
vvp cache_sim
gtkwave cache_tb.vcd
