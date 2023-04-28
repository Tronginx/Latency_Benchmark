#!/bin/bash

# Output file name
output_file="private_latency_summary.csv"

# Remove the existing output file
rm -f "$output_file"

# Compile the program
gcc -o latency latency_benchmark.c

# Write the header to the CSV file
echo "Message size (bytes),Average latency (us),25th percentile latency (us),50th percentile latency (us),75th percentile latency (us),90th percentile latency (us),99th percentile latency (us),99.9th percentile latency (us)" >> "$output_file"

# Loop through powers of 2
for (( power=3; power<=15; power++ )); do
  num=$((2 ** power))
  
  echo "Running program with: $num bytes"
  
  # Run the program and capture the output
  output=$(./latency sender $num)
  
  # Extract the latency values from the output
  average=$(echo "$output" | grep -oP '(?<=Average latency: ).*')
  p25=$(echo "$output" | grep -oP '(?<=25th percentile latency: ).*')
  p50=$(echo "$output" | grep -oP '(?<=50th percentile latency: ).*')
  p75=$(echo "$output" | grep -oP '(?<=75th percentile latency: ).*')
  p90=$(echo "$output" | grep -oP '(?<=90th percentile latency: ).*')
  p99=$(echo "$output" | grep -oP '(?<=99th percentile latency: ).*')
  p99_9=$(echo "$output" | grep -oP '(?<=99.9th percentile latency: ).*')

  # Write the values to the CSV file
  echo "$num,$average,$p25,$p50,$p75,$p90,$p99,$p99_9" >> "$output_file"
done

echo "Execution completed."
