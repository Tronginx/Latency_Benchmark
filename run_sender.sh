#!/bin/bash
output_file="sender_output.csv"

echo "Message Size (bytes),Average Latency (ms)" > $output_file

for i in {1..16000}; do
    echo "Running test for message size: $i bytes"
    output=$(./latency_benchmark sender $i)
    echo "$i,$output" >> $output_file
    echo "Test completed for message size: $i bytes"
done

echo "All tests completed."

~                                                                               
~                                                                               
~                                                                               
~                                                                               
~                                                                               
~                                                                               
~                                                                               
~                                                                               
~                                                                               
"run_sender.sh" 14L, 355C                                     14,0-1        All
