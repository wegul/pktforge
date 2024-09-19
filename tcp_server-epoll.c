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
int epfd;

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

    int bytes_recvd = 0;
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


// Return a epoll_fd; 
// param is sock to be listened
// Note that this epoll should listen to both 'accept'(servfd) and 'recv'(clifd)
void do_add_event(int sock) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
}
void do_del_event(int sock) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_DEL, sock, &ev);
}



int main(int argc, char* argv[]) {
    int server_fd, client_fd, conn_fin = 0;
    struct epoll_event events[MAX_CONN];// manages all connections
    struct Stat st;
    rcvbuf = (uint8_t*)malloc(BUFFER_SIZE);
    epfd = epoll_create(1);

    server_fd = do_bind_server_sock();
    do_listen(server_fd);
    do_add_event(server_fd);

    uint64_t total_bytes_recvd = 0;
    double total_time_taken = 0.0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        int nr_ready = epoll_wait(epfd, events, MAX_CONN, -1);
        if (nr_ready < -1) {
            perror("nready<0");
            break;
        }

        for (size_t i = 0; i < nr_ready; i++) {
            if (events[i].data.fd == server_fd)// This is accept event
            {
                client_fd = do_accept(server_fd);
                do_add_event(client_fd);
            }
            else {
                client_fd = events[i].data.fd;
                st = do_recv(client_fd);
                if (st.bytes <= 0) {
                    printf("Finished client socket <%d>\n", client_fd);
                    do_close(client_fd);
                    do_del_event(client_fd);
                    conn_fin++;
                }
                else {
                    total_bytes_recvd += st.bytes;
                    if (total_bytes_recvd >= ONEGB * 2) {
                        st.start = start;
                        total_time_taken = cal_timetaken(start, st.end);
                        printf("bytes=%ld, timesec=%.2f\n", total_bytes_recvd / ONEGB, total_time_taken);
                        printf("_____\n Per 2 GB xput=%.8f\n", (total_bytes_recvd / ONEGB) / total_time_taken * 8);
                        total_bytes_recvd = 0;
                        total_time_taken = 0;
                        clock_gettime(CLOCK_MONOTONIC, &start);
                    }
                }
            }
        }
        // if (conn_fin >= NR_CONN) {
        //     printf("_____\n %.8f\n", total_bytes_recvd / total_time_taken * 8);
        //     conn_fin = 0;
        // }

    }
    // printf("_____\n %.8f\n", total_bytes_recvd / total_time_taken * 8);
    do_close(server_fd);
}
