#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


#define EPOCH 10
#define FSIZE 1024 * 1024 * 512 //512MB



char* serv_ip = "10.0.1.4";
int PORT = 8080;
const size_t DATA_SIZE = (size_t)FSIZE;
const size_t SNDBUF_SIZE = 2 * DATA_SIZE;
uint8_t* data;
int cur_epoch = 0;


int mp_connect() {
    int sock = 0;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t addr_len = sizeof(cli_addr);


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &SNDBUF_SIZE, sizeof(SNDBUF_SIZE));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    getsockname(sock, (struct sockaddr*)&cli_addr, &addr_len);
    int client_port = (int)ntohs(cli_addr.sin_port);
    printf("connected! local port= %d\n", client_port);
    //Warm up
    send(sock, data, DATA_SIZE / 2, 0);

    return sock;
}
// Returns bytes sent
int mp_send(int sock, int flag) {
    double time_taken, xput;

    struct timespec start, end;
    size_t bytes_sent = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    bytes_sent = send(sock, data, DATA_SIZE, flag);
    clock_gettime(CLOCK_MONOTONIC, &end);

    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;

    xput = (bytes_sent / (1024.0 * 1024.0 * 1024.0)) / time_taken;  // MB/s
    // printf("Total data sent: %ld MB\n", bytes_sent / (1024 * 1024));
    // printf("Time taken: %.6f seconds\n", time_taken);
    // printf("In epoch <%d>, Xput= %.8f Gbps\n", cur_epoch, xput * 8);
    return bytes_sent;
}


int main(int argc, char* argv[]) {
    int opt;
    int nr_conn = 2;

    data = (uint8_t*)malloc(DATA_SIZE);
    memset(data, 'A', DATA_SIZE);


    while ((opt = getopt(argc, argv, "s:n:P:")) != -1) {
        switch (opt) {
        case 's':
            serv_ip = optarg;
            break;
        case 'n':
            char* PORT_chars = optarg;
            PORT = strtol(PORT_chars, NULL, 10);
            break;
        case 'P':
            nr_conn = strtol(optarg, NULL, 8);
            break;
        default:
            break;
        }
    }


    pthread_t* threads = (pthread_t*)malloc(nr_conn * sizeof(pthread_t));
    int* sock_fd = (int*)malloc(nr_conn * sizeof(int));
    for (size_t i = 0; i < nr_conn; i++) {
        sock_fd[i] = mp_connect();
    }

    while (cur_epoch++ < EPOCH) {
        int total_sent = 0;
        double time_taken, xput;
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (size_t i = 0; i < nr_conn - 1; i++) {
            total_sent += mp_send(sock_fd[i], 0);//non-blocking
        }
        total_sent += mp_send(sock_fd[nr_conn - 1], 0);//blocking
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken = (end.tv_sec - start.tv_sec) * 1e9;
        time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        xput = (total_sent / (1024.0 * 1024.0 * 1024.0)) / time_taken;  // MB/s
        printf("===In epoch <%d>, Average Xput= %.8f Gbps\n", cur_epoch, xput * 8);
    }

    for (size_t i = 0; i < nr_conn; i++) {
        close(sock_fd[i]);
    }
    return 0;
}
