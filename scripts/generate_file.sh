#!/bin/bash

# Check if the user has provided an argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <size in GB>"
    exit 1
fi

# Size in GB from the first command-line argument
size_gb=$1

# File name for the generated file
file_name="largefile.bin"

# Create a file of specified size using dd
echo "head -c ${size_gb}G /dev/urandom > $file_name"
head -c ${size_gb}G /dev/urandom > $file_name

echo "Generated file '$file_name' of size ${size_gb}GB."