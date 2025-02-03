#!/bin/bash

# Input file name
input_file="$1"

# Output C file
output_file="explicit_test.c"

# Initialize the output file
echo "#include \"explicit.h\"" > $output_file
echo "#include <stdio.h>" >> $output_file
echo "" >> $output_file
echo "int main() {" >> $output_file

# Read through each line of the input file
while IFS= read -r line
do
    # Split the line into components
    command=$(echo $line | awk '{print $1}')
    case $command in
        init)
            # init <heap_size>
            heap_size=$(echo $line | awk '{print $2}')
            echo "    exp_heap_init($heap_size);" >> $output_file
            ;;
        a)
            # a <id-number> <size>
            id=$(echo $line | awk '{print $2}')
            size=$(echo $line | awk '{print $3}')
            echo "    void* p$id = exp_malloc($size);" >> $output_file
            ;;
        r)
            # r <id-number> <size>
            id=$(echo $line | awk '{print $2}')
            size=$(echo $line | awk '{print $3}')
            echo "    p$id = exp_realloc(p$id, $size);" >> $output_file
            ;;
        f)
            # f <id-number>
            id=$(echo $line | awk '{print $2}')
            echo "    exp_free(p$id);" >> $output_file
            ;;
        destroy)
            # destroy <heap_size>
            heap_size=$(echo $line | awk '{print $2}')
            echo "    exp_heap_destroy();" >> $output_file
            ;;
        iterate)
            # iterate
            echo "    iterate_heap();" >> $output_file
            ;;
        *)
            # Handle invalid command
            echo "Unknown command: $command"
            ;;
    esac
done < "$input_file"

# Close the main function
echo "    return 0;" >> $output_file
echo "}" >> $output_file

echo "C file generated: $output_file"

