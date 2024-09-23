#!/bin/bash
# Given a $OUTNAME, extract the perf.data to generate a svg.

FLG_PATH="/home/weigao/FlameGraph"

# Check if the user has provided an argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <output file name.svg>"
    exit 1
fi
OUTNAME=$1

set -e

# Move "perf.data" to "out.perf" in ./data folder
echo "perf script > ./data/out.perf"
perf script > ./data/out.perf

# Use flamegraph fold stack
echo "${FLG_PATH}/stackcollapse-perf.pl ./data/out.perf > ./data/out.folded"
${FLG_PATH}/stackcollapse-perf.pl ./data/out.perf > ./data/out.folded

# Generate svg
echo "${FLG_PATH}/flamegraph.pl ./data/out.folded > ./data/${OUTNAME}.svg"
${FLG_PATH}/flamegraph.pl ./data/out.folded > ./data/${OUTNAME}.svg