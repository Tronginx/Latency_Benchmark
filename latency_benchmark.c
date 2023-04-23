#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define PORT 8888
#define IP_SENDER "128.105.144.165"
#define IP_RECEIVER "128.105.144.160"
#define NUM_TRIALS 1000

int MESSAGE_SIZE = 1024;

double measure_latency(int sock, struct sockaddr_in *addr, int message_size) {
    char *message = (char *)malloc(message_size * sizeof(char));
    memset(message, 'A', message_size);

    char buffer[message_size];
    struct timeval start, end;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    gettimeofday(&start, NULL);
    sendto(sock, message, message_size, 0, (struct sockaddr *)addr, sizeof(*addr));
    recvfrom(sock, buffer, message_size, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    gettimeofday(&end, NULL);

    free(message);

    double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken += (end.tv_usec - start.tv_usec);
    return time_taken / 1000;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s sender|receiver [message_size]\n", argv[0]);
        exit(1);
    }

    if (argc == 3) {
        MESSAGE_SIZE = atoi(argv[2]);
    }

    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // Set the socket receive buffer size
    int receive_buffer_size = MESSAGE_SIZE * 2;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &receive_buffer_size, sizeof(receive_buffer_size)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (strcmp(argv[1], "sender") == 0) {
        server_addr.sin_addr.s_addr = inet_addr(IP_RECEIVER);

        double total_latency = 0;
        for (int i = 0; i < NUM_TRIALS; i++) {
            total_latency += measure_latency(sock, &server_addr, MESSAGE_SIZE);
        }

        printf("Average latency: %.2f ms\n", total_latency / NUM_TRIALS);
    } else if (strcmp(argv[1], "receiver") == 0) {
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind");
            exit(1);
        }

        char buffer[MESSAGE_SIZE];
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        while (1) {
            recvfrom(sock, buffer, MESSAGE_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
            sendto(sock, buffer, MESSAGE_SIZE, 0, (struct sockaddr *)&client_addr, client_addr_len);
        }
    } else {
        fprintf(stderr, "Invalid option. Use 'sender' or 'receiver'.\n");
        exit(1);
    }

    close(sock);
    return 0;
}

