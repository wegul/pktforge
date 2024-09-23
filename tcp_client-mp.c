#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include "utils.h"

int PORT = 8888;
uint8_t* data;
char* serv_ip = "10.0.1.4";

//return a client fd
int do_conn() {
    int client_fd = 0;
    const size_t DATA_SIZE = (size_t)BUFFER_SIZE;
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
    setsockopt(client_fd, SOL_SOCKET, SO_SNDBUF, &DATA_SIZE, sizeof(DATA_SIZE));

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    return client_fd;
}

struct Stat do_send(int client_fd) {
    struct timespec start, end;
    int bytes_sent = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bytes_sent = send(client_fd, data, BUFFER_SIZE, 0);
    clock_gettime(CLOCK_MONOTONIC, &end);

    struct Stat st;
    st.bytes = bytes_sent;
    st.start = start;
    st.end = end;

    return st;
}
void do_close(int sock) {
    close(sock);
}

void do_transfer(int client_fd) {
    struct Stat st = do_send(client_fd);
    // if (st.bytes > 0) {
    //     double xput = cal_xput(st);
    //     printf("Send xput is %.8f\n", xput);
    // }
    // else {
    //     printf("Send error\n");
    // }
}
void* threadFunc(void* client_fd_ptr) {

    int client_fd = *(int*)client_fd_ptr;
    int cur_epoch = EPOCH;
    while (cur_epoch--) {
        do_transfer(client_fd);
    }
}
int main(int argc, char* argv[]) {
    int opt;
    const size_t DATA_SIZE = (size_t)BUFFER_SIZE;

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
    data = (uint8_t*)malloc(DATA_SIZE);
    memset(data, 'A', DATA_SIZE);


    // int client_fd = do_conn();
    int client_fds[NR_CONN] = { 0 };
    pthread_t threads[NR_CONN];

    for (size_t i = 0; i < NR_CONN; i++) {
        client_fds[i] = do_conn();
        pthread_create(&threads[i], NULL, threadFunc, &client_fds[i]);
    }
    for (size_t i = 0; i < NR_CONN; i++) {
        pthread_join(threads[i], NULL);
        do_close(client_fds[i]);
    }

    // printf("_____\n %.8f\n", total_xput / cnt);
    // do_close(client_fd);


    return 0;
}