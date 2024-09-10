#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024 * 1024 * 64  // 64 MB

int mp_receive(int remote_sock) {
    uint8_t* buffer = (uint8_t*)malloc(BUFFER_SIZE);
    struct timespec start, end;
    int bytes_recvd, total_recvd = 0;
    double total_time = 0, time_taken, xput;
    while (1) {
        bytes_recvd = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        bytes_recvd += recv(remote_sock, buffer, BUFFER_SIZE, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (bytes_recvd <= 0) {
            break;
        }
        time_taken = (end.tv_sec - start.tv_sec) * 1e9;
        time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        // xput = (bytes_recvd / (1024.0 * 1024.0 * 1024.0)) / time_taken;  // GB/s
        // printf("Recv Xput= %.8f Gbps\n", xput * 8);

        total_time += time_taken;
        total_recvd += bytes_recvd;
        if (total_recvd >= 1024 * 1024 * 512) {
            xput = (total_recvd / (1024.0 * 1024.0 * 1024.0)) / total_time;  // GB/s
            printf("AVG Recv Xput= %.8f Gbps\n", xput * 8);
            total_time = 0;
            total_recvd = 0;
        }
    }
    close(remote_sock);
    free(buffer);
}
void* threadFunc(void* vargp) {
    int* remote_sock = vargp;
    struct sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(remote_addr);
    getpeername(*remote_sock, (struct sockaddr*)&remote_addr, &addr_len);

    // Convert IP addresses from binary to text form
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    inet_ntop(AF_INET, &(remote_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    client_port = (int)ntohs(remote_addr.sin_port);
    printf("New connection from %s:%d\n", client_ip, client_port);
    mp_receive(*remote_sock);
    printf("Finish connection %s:%d\n", client_ip, client_port);
    return NULL;
}
int main(int argc, char* argv[]) {
    const size_t RCVBUF_SIZE = BUFFER_SIZE;
    int server_fd;
    struct sockaddr_in local_addr;
    int nr_conn = 1, cur_conn = 0;
    int opt = 1;
    int addrlen = sizeof(local_addr);


    while ((opt = getopt(argc, argv, "P:")) != -1) {
        switch (opt) {
        case 'P':
            nr_conn = strtol(optarg, NULL, 8);
            break;
        default:
            break;
        }
    }

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &RCVBUF_SIZE, sizeof(RCVBUF_SIZE));

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(PORT);

    // Binding socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pthread_t* threads = (pthread_t*)malloc(nr_conn * sizeof(pthread_t));
    while (cur_conn < nr_conn) {
        struct sockaddr_in remote_addr;
        int remote_sock;
        printf("Waiting for connection\n");

        if ((remote_sock = accept(server_fd, (struct sockaddr*)&remote_addr, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // start threading
        pthread_create(&threads[cur_conn++], NULL, threadFunc, &remote_sock);
    }
    for (int i = 0; i < nr_conn; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("________\nFinished all.\n");

    close(server_fd);
    free(threads);

    return 0;
}
