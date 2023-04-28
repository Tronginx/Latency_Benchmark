#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 32768
#define EPOLL_EVENTS 1

int main() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("Receiver is running on port %d\n", PORT);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev, events[EPOLL_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = sock;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, EPOLL_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(1);
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sock) {
                char buffer[MAX_BUFFER_SIZE];
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);

                ssize_t received_len = recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
                if (received_len < 0) {
                    perror("recvfrom");
                    exit(1);
                }

                sendto(sock, buffer, received_len, 0, (struct sockaddr *)&client_addr, client_addr_len);
            }
        }
    }

    close(sock);
    return 0;
}
