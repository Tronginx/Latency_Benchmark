# Latency Benchmark

This C program is a simple latency benchmark tool that measures the round-trip time (RTT) or latency between a sender and a receiver using UDP sockets. The program can be run in two modes: sender and receiver.

## Prerequisites

The program requires a Linux environment with the following headers:

- stdio.h
- stdlib.h
- string.h
- sys/socket.h
- arpa/inet.h
- unistd.h
- sys/time.h
- time.h
## Configuration

Before running the program, you need to set the sender and receiver IP addresses in the `#define` statements at the beginning of the source code:

```c
#define IP_SENDER ""
#define IP_RECEIVER ""
```

Replace the empty strings with the corresponding IP addresses.

You can also adjust the number of trials and the default message size:

```c
#define NUM_TRIALS 1000
int MESSAGE_SIZE = 1024;
```
## Usage

**Notice:** Make sure you have configured the program before compilation and using it.

Compile the program using the following command:

```
gcc -o latency_benchmark latency_benchmark.c
```

Run the program in receiver mode on the machine acting as the receiver:

```
./latency_benchmark receiver [message_size_bytes]
```

Run the program in sender mode on the machine acting as the sender:

```
./latency_benchmark sender [message_size_bytes]
```

The message size is in bytes.

**Notice:** You may need to run the receiver before the sender.

## Example

My `latency_benchmark.c` is located at `/users/latency_benchmark.c` on both nodes.

First, compile the program using the command on both nodes/servers:

```
gcc -o latency_benchmark latency_benchmark.c
```

Assuming we have two nodes: node 0 and node 1.

In node0: I will run the receiver on node 0. The following command specifies the message size as 1,600 bytes:

```
./latency_benchmark receiver 1600
```

I will run the sender on node 1 using the following command:

```
./latency_benchmark sender 1600
```

The sender will print the average latency over the specified number of trials:

```
Average latency: 0.21 ms
```

## How it works

The program measures the latency by sending a message of a specified size from the sender to the receiver. The receiver then sends the message back to the sender. The time taken for this round-trip is calculated and averaged over multiple trials to obtain a more accurate result.

The program uses UDP sockets, which are connectionless and do not guarantee delivery or message ordering. This makes the program suitable for measuring latency in networks where packet loss and out-of-order delivery are possible.



