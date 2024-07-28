#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>

#define PORT 8080
#define ITERATION 1
#define BUFFER_SIZE 1024 * 1024 * 128  // 128 MB


int main() {
    const size_t DATA_SIZE = (size_t)1024 * 1024 * 1024 * 8;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    uint8_t* buffer = (uint8_t*)malloc(BUFFER_SIZE);

    size_t total_received = 0;
    size_t bytes_received;



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

    while (1) {
        if (recv(new_socket, buffer, BUFFER_SIZE, MSG_WAITALL) <= 0) {
            break;
        }
    }

    // // Start measurement
    // total_received = 0;
    // bytes_received = 0;
    // while (total_received < DATA_SIZE) {
    //     bytes_received = recv(new_socket, buffer, BUFFER_SIZE, MSG_MORE);
    //     if (bytes_received <= 0) {
    //         break;
    //     }
    //     total_received += bytes_received;
    // }

    printf("Finish all\n");

    close(new_socket);
    close(server_fd);
    free(buffer);


    return 0;
}