#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>

#define PORT 8080
#define BUFFER_SIZE 1024 * 1024 * 512  // 1GB

int do_recv() {

}

int main() {
    const size_t RCVBUF_SIZE = BUFFER_SIZE;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    uint8_t* buffer = (uint8_t*)malloc(BUFFER_SIZE);

    struct timespec start, end;
    int bytes_recvd, total_recvd = 0;
    double total_time = 0, time_taken, xput;


    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &RCVBUF_SIZE, sizeof(RCVBUF_SIZE));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for connection\n");

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    // Convert IP addresses from binary to text form
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("New connection from %s\n", client_ip);

    double total_xput = 0;
    int counter = 0;
    while (1) {
        bytes_recvd = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        bytes_recvd = recv(new_socket, buffer, BUFFER_SIZE, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (bytes_recvd <= 0) {
            break;
        }
        time_taken = (end.tv_sec - start.tv_sec) * 1e9;
        time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        xput = (bytes_recvd / (1024.0 * 1024.0 * 1024.0)) / time_taken;  // GB/s
        // printf("%.8f \n", xput * 8);
        total_time += time_taken;
        total_recvd += bytes_recvd;
        total_xput += xput;
        counter++;
        // if (counter % 100 == 0) {
        //     xput = (total_recvd / (1024.0 * 1024.0 * 1024.0)) / total_time;  // GB/s
        //     printf("AVG Recv Xput= %.8f Gbps\n", xput * 8);
        //     total_time = 0;
        //     total_recvd = 0;
        // }
    }
    xput = total_xput / counter;  // GB/s
    printf("______\n Total AVG Recv Xput= %.8f Gbps\n", xput * 8);

    printf("Finish all\n");

    close(new_socket);
    close(server_fd);
    free(buffer);


    return 0;
}