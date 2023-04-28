# Latency Benchmark

This repository contains a network latency benchmark program that measures the latency between private and public interfaces using User Datagram Protocol (UDP). The program incorporates asynchronous IO with epoll, calculates average latency, tail latency (percentiles), and provides detailed measurements for different message sizes.

## Prerequisites

- Linux-based operating system
- C compiler (e.g., GCC)

## Program Description

The latency benchmark program performs the following tasks:

- Measures network latency in both private and public interfaces.
- Utilizes asynchronous IO with epoll for efficient handling of incoming data.
- Sends UDP packets with various message sizes (power of 2) and a specified number of trials(default: 1 million times).
- Attaches a timestamp to each message to calculate round-trip latency accurately.
- Records latency values for each trial and message size.
- Computes average latency and tail latency (percentiles) based on the recorded data.
- Prints the results for analysis.

## Results

The benchmark program provides detailed measurements for each message size, including average latency, 25th percentile latency, 50th percentile latency, 75th percentile latency, 90th percentile latency, 99th percentile latency, and 99.9th percentile latency.
