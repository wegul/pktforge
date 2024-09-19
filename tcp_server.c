#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include "utils.h"



uint8_t* rcvbuf;

int do_bind_server_sock() {
    const size_t RCVBUF_SIZE = BUFFER_SIZE;
    struct sockaddr_in srv_addr;
    int server_fd;
    int opt = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8888
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &RCVBUF_SIZE, sizeof(RCVBUF_SIZE))) {
        perror("setsockopt, rcvsize");
        exit(EXIT_FAILURE);
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(8888);

    // Binding socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void do_listen(int server_fd) {
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for connection\n");
}

int do_accept(int server_fd) {
    int client_fd;
    struct sockaddr_in cli_addr;
    int addrlen = sizeof(cli_addr);

    if ((client_fd = accept(server_fd, (struct sockaddr*)&cli_addr, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    // Convert IP addresses from binary to text form
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("New connection from %s, sock <%d>\n", client_ip, client_fd);

    return client_fd;
}
// Return received bytes and time consumption.
struct Stat do_recv(int client_fd) {
    struct timespec start, end;

    uint64_t bytes_recvd = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bytes_recvd = recv(client_fd, rcvbuf, BUFFER_SIZE, 0);
    clock_gettime(CLOCK_MONOTONIC, &end);

    struct Stat st;
    st.bytes = bytes_recvd;
    st.start = start;
    st.end = end;

    return st;
}

void do_close(int sock) {
    close(sock);
}


int main(int argc, char* argv[]) {
    int server_fd, client_fd = 0;
    uint64_t total_bytes_recvd = 0;
    double total_time_taken = 0.0;
    struct Stat st;
    rcvbuf = (uint8_t*)malloc(BUFFER_SIZE);
    server_fd = do_bind_server_sock();
    do_listen(server_fd);
    while (1) {
        client_fd = do_accept(server_fd);

        while (client_fd > 0) {
            st = do_recv(client_fd);
            if (st.bytes <= 0) {
                printf("Finished client socket <%d>\n", client_fd);
                do_close(client_fd);
                client_fd = -1;
            }
            else {
                double xput = cal_xput(st);
                // printf("Recv xput is %.8f\n", xput);
                total_bytes_recvd += st.bytes;
                total_time_taken += cal_time(st);
                if (total_bytes_recvd >= 2 * ONEGB) {
                    printf("_____\n Per 2 GB xput=%.8f\n", (total_bytes_recvd / ONEGB) / total_time_taken * 8);
                    total_bytes_recvd = 0;
                    total_time_taken = 0;
                }
            }

        }
    }

    do_close(server_fd);
}

