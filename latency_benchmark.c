#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#define PORT 8888
#define IP_RECEIVER ""
#define MAX_TRIALS 1000000
#define EPOLL_MAX_EVENTS 1000

uint64_t get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

void print_progress(int current, int max) {
    int bar_width = 50;
    double progress = (double)current / max;
    int progress_bar = progress * bar_width;

    printf("\r[");
    for (int i = 0; i < bar_width; ++i) {
        if (i < progress_bar) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    printf("] %3d%%", (int)(progress * 100));
    fflush(stdout);
}

void set_non_blocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        exit(1);
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(1);
    }
}
int compare_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;

    if (da > db) {
        return 1;
    } else if (da < db) {
        return -1;
    } else {
        return 0;
    }
}

double measure_latency(int sock, struct sockaddr_in *addr, int message_size) {
    char *message = (char *)malloc(message_size);
    if (!message) {
        perror("malloc");
        exit(1);
    }
    memset(message, 'A', message_size - sizeof(uint64_t));

    uint64_t *timestamp = (uint64_t *)(message + message_size - sizeof(uint64_t));
    *timestamp = get_timestamp();

    if (sendto(sock, message, message_size, 0, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        perror("sendto");
        exit(1);
    }

    char recv_buffer[message_size];
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);

    ssize_t received_len = recvfrom(sock, recv_buffer, message_size, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
    if (received_len < 0) {
        perror("recvfrom");
        exit(1);
    }

    if (received_len != message_size) {
        fprintf(stderr, "Received message size (%zd) does not match the expected size (%d)\n", received_len, message_size);
        exit(1);
    }

    uint64_t recv_timestamp = *(uint64_t *)(recv_buffer + received_len - sizeof(uint64_t));
    double latency = (double)(get_timestamp() - recv_timestamp) / 2;

    free(message);

    return latency;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s sender [message_size]\n", argv[0]);
        exit(1);
    }

    int message_size = atoi(argv[2]);

    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_RECEIVER);
    server_addr.sin_port = htons(PORT);

    set_non_blocking(sock);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    double latencies[MAX_TRIALS] = {0};
    int sent_count = 0;
    int received_count = 0;

    while (received_count < MAX_TRIALS) {
        if (sent_count < MAX_TRIALS) {
            char *message = (char *)malloc(message_size);
            if (!message) {
                perror("malloc");
                exit(1);
            }
            memset(message, 'A', message_size - sizeof(uint64_t));

            uint64_t *timestamp = (uint64_t *)(message + message_size - sizeof(uint64_t));
            *timestamp = get_timestamp();

            if (sendto(sock, message, message_size, 0, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("sendto");
                    exit(1);
                }
            } else {
                ++sent_count;
            }
        }

        struct epoll_event events[EPOLL_MAX_EVENTS];
        int nfds = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, 100);

        for (int i = 0; i < nfds; ++i) {
            if (events[i].events & EPOLLIN) {
                char recv_buffer[message_size];
                struct sockaddr_in recv_addr;
                socklen_t recv_addr_len = sizeof(recv_addr);

                ssize_t received_len = recvfrom(sock, recv_buffer, message_size, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
                if (received_len < 0) {
                    perror("recvfrom");
                    exit(1);
                }

                if (received_len != message_size) {
                    fprintf(stderr, "Received message size (%zd) does not match the expected size (%d)\n", received_len, message_size);
                    exit(1);
                }

                uint64_t recv_timestamp = *(uint64_t *)(recv_buffer + received_len - sizeof(uint64_t));
                double latency = (double)(get_timestamp() - recv_timestamp) / 2;

                latencies[received_count++] = latency;
            }
        }

        // Print progress bar, comment out when using shell script
        print_progress(received_count, MAX_TRIALS);
    }
    // Compute average latency
    double total_latency = 0;
    for (int i = 0; i < MAX_TRIALS; i++) {
        total_latency += latencies[i];
    }
    printf("\nAverage latency: %.2f \n", total_latency / MAX_TRIALS);

    // Compute tail latency
    qsort(latencies, MAX_TRIALS, sizeof(double), compare_double);
    printf("25th percentile latency: %.2f \n", latencies[(int)(0.25 * MAX_TRIALS)]);
    printf("50th percentile latency: %.2f \n", latencies[(int)(0.50 * MAX_TRIALS)]);
    printf("75th percentile latency: %.2f \n", latencies[(int)(0.75 * MAX_TRIALS)]);
    printf("90th percentile latency: %.2f \n", latencies[(int)(0.90 * MAX_TRIALS)]);
    printf("99th percentile latency: %.2f \n", latencies[(int)(0.99 * MAX_TRIALS)]);
    printf("99.9th percentile latency: %.2f \n", latencies[(int)(0.999 * MAX_TRIALS)]);

    close(sock);
    return 0;
}
