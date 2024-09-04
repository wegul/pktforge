CC=gcc
CFLAGS=-I -g -O2

# Common dependencies
DEPS = 

# Object files for each application
OBJ_APP1 = tcp_server.o 
OBJ_APP2 = tcp_client.o 

# Default target
all: server client

# Compile rules for object files
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Linking rules for each application
server: $(OBJ_APP1)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(OBJ_APP2)
	$(CC) -o $@ $^ $(CFLAGS)

# Phony targets for cleanliness
.PHONY: clean

clean:
	rm -f *.o server client