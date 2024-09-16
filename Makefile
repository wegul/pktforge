CC=gcc
CFLAGS=-I -g -O2

# Common dependencies
DEPS = 

# Object files for each application
OBJ_SERV = tcp_server.o 
OBJ_CLI = tcp_client.o 
OBJ_SERV_EP = tcp_server-epoll.o
OBJ_CLI_MP = tcp_client-mp.o

# Default target
all: server client server-ep client-mp

# Compile rules for object files
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Linking rules for each application
server: $(OBJ_SERV)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(OBJ_CLI)
	$(CC) -o $@ $^ $(CFLAGS)

server-ep: $(OBJ_SERV_EP)
	$(CC) -o $@ $^ $(CFLAGS)

client-mp: $(OBJ_CLI_MP)
	$(CC) -o $@ $^ $(CFLAGS)


# Phony targets for cleanliness
.PHONY: clean

clean:
	rm -f *.o server client server-ep client-mp