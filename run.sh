#!/bin/bash
gcc ./tcp_client.c -o client -O2
gcc ./tcp_server.c -o server -O2

# Task 1
task1() {
  echo "./server"
  ./server
}
# Task 2
task2() {
 echo "./client"
  ./client
}

# Run both tasks in the background
task1 > task1_output.txt 2>&1 &
pid1=$!
sleep 10
task2 > task2_output.txt 2>&1 &
pid2=$!

# Wait for both tasks to complete
wait $pid1
wait $pid2

# Print the output of both tasks
echo "Output of Task 1:"
cat task1_output.txt

echo "Output of Task 2:"
cat task2_output.txt

