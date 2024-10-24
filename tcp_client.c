#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include "utils.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

int PORT = 8888;
char* data;
int payload_fd;
char* serv_ip = "10.0.1.4";

uint64_t total_bytes_sent = 0;

//return a client fd
int do_conn() {
    int client_fd = 0;
    const size_t SND_BUF_SIZE = (size_t)ONEGB;
    struct sockaddr_in serv_addr;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    //Set send buf
    if (setsockopt(client_fd, SOL_SOCKET, SO_SNDBUF, &SND_BUF_SIZE, sizeof(SND_BUF_SIZE))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // getsockopt(client_fd,SOL_SOCKET,SO_SNDBUF)

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    return client_fd;
}

struct Stat do_send(int client_fd) {
    struct timespec start, end;
    uint64_t bytes_sent = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bytes_sent += write(client_fd, data, ONEGB);
    clock_gettime(CLOCK_MONOTONIC, &end);

    struct Stat st;
    st.bytes = bytes_sent;
    st.start = start;
    st.end = end;

    return st;
}


void do_transfer(int client_fd) {
    struct Stat st;
    st = do_send(client_fd);
    if (st.bytes > 0) {
        double xput = cal_xput(st);
        printf("Send xput is %.8f\n", xput);
    }
    else {
        perror("Send error\n");
        exit(EXIT_FAILURE);
    }
}
void loop_transfer(int client_fd) {
    int cur_epoch = EPOCH;
    while (cur_epoch > 0) {
        // do_send(client_fd);
        do_transfer(client_fd);
        cur_epoch--;
    }
}
void do_close(int sock) {
    close(sock);
}
int main(int argc, char* argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "s:p:")) != -1) {
        switch (opt) {
        case 's':
            serv_ip = optarg;
            break;
        case 'p':
            char* PORT_chars = optarg;
            PORT = strtol(PORT_chars, NULL, 10);
            break;
        default:
            break;
        }
    }
    payload_fd = open("./scripts/largefile.bin", O_RDONLY);
    if (payload_fd < 0) {
        perror("Failed to obtain file descriptor");
        return -1;
    }
    data = (int8_t*)mmap(NULL, ONEGB, PROT_READ, MAP_PRIVATE, payload_fd, 0);
    // data = (int8_t*)malloc(WND_SIZE);
    // memset(data, 'A', WND_SIZE);

    int client_fd = do_conn();
    loop_transfer(client_fd);
    do_close(client_fd);

    return 0;
}