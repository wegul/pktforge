#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5201

const char* serv_ip = "10.0.0.4";

int main() {
    const size_t DATA_SIZE = (size_t)1024 * 1024 * 1024 * 8;

    uint8_t* data = (uint8_t*)malloc(DATA_SIZE);
    uint8_t* warmup_data = (uint8_t*)malloc(DATA_SIZE / 4);
    double time_taken, xput, total_time = 0;
    memset(data, 'A', DATA_SIZE);
    memset(warmup_data, 'A', DATA_SIZE / 4);


    int sock = 0;
    struct sockaddr_in serv_addr;
    size_t total_sent = 0, bytes_sent = 0;
    size_t bytes_received;
    struct timespec start, end;

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

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    //Warm up
    send(sock, warmup_data, DATA_SIZE / 4, MSG_WAITALL);

    while (total_sent < DATA_SIZE) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        bytes_sent = send(sock, data, DATA_SIZE, MSG_WAITALL);
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_sent += bytes_sent;
        time_taken = (end.tv_sec - start.tv_sec) * 1e9;
        time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        total_time += time_taken;

        xput = (bytes_sent / (1024.0 * 1024.0 * 1024.0)) / time_taken;  // MB/s
        // printf("Total data sent: %ld MB\n", bytes_sent / (1024 * 1024));
        // printf("Time taken: %.6f seconds\n", time_taken);
        printf("Xput: %.8f Gbps\n", xput * 8);
    }
    xput = (total_sent / (1024.0 * 1024.0 * 1024.0)) / total_time;  // MB/s

    printf("\nAvg Xput: %.8f Gbps\n", xput * 8);


    close(sock);

    return 0;
}