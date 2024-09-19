#!/bin/bash

# Set the IP address and port of the destination
DEST_IP="10.0.0.4"
DEST_PORT="5201"

DATA=$(printf '%*s' 8800 | tr ' ' 'A')

# Connect to the destination and keep the connection open
{
    # Feed data into the connection indefinitely
    while true; do
	    echo "Sending data at $(date): ${DATA}"  # Send a timestamp or any data
        sleep 0.001  # Wait a bit before sending the next line
    done
} | nc $DEST_IP $DEST_PORT
