# pktforge
Creates TCP packets with a specific payload.
Bulk transfer.


### Run:

    make
    
Server side (default port=8888): 

    ./server

Client side:

    ./client -s <ip> -p <port>

======================
======================

## Flow steering instruction

Since Corundum does not have aRFS. We have to rely on RSS + some manual hacking.

So we will set:   
`sudo ethtool -X <$IF> equal 1`   
such that there is only one active rx_queue. Then we run:   
`sudo set_irq_affinity.sh $INTF`,   
which helps set affinity to IRQs such that only one CPU will do the napi_polling for the rx_queue. Here we need to pay attention to the range of IRQ number (pick the smallest).

However, this script does not care about NUMA locality. Hence next, we do `lstopo` to see which CPUs are on the same socket with our NIC. Lastly, we manually set `/proc/irq/<#>/smp_affinity` to a NUMA-friendly CPU, e.g.,   
`echo 00,00000000,00000002 > /proc/irq/285/smp_affinity`.   
This means only cpu_1 will process IRQ-285.

we run an experiment while watching `htop`, in order to get to the queue-mapped CPU#.

After that, we can run `taskset -c <CPU_ID> iperf3/server/client...`. Note that we have to find out the correct CPU for both sender and receiver. In addition, we can make it the highest priority by:   
`sudo nice -n -20 taskset -c <CPU_ID> iperf3/server/client...`.

======================
======================

# perf tool

`sudo perf record -g -C <#> [command]`  
-a: monitor all CPUs  
-g: generate graphic data  
-C: capture only CPU_id  

Convert 'perf.data' to text: `perf script > out.perf`


## iPerf command
For MLNX: 
`sudo perf record -a -g -C 0 nice -n -20 taskset -c 0 iperf3 -c 10.0.1.3 -t100`;   
`sudo perf record -a -g -C 0 nice -n -20 taskset -c 0 iperf3 -s`

For CRDM: the CPU id is '1' 

## FTP command
MLNX:
- sudo perf record -a -g -C 0 nice -n -20 taskset -c 0 ./client -s 10.0.1.3

- sudo perf record -a -g -C 0 nice -n -20 taskset -c 0 ./server 

CRDM:
- sudo perf record -a -g -C 0 nice -n -20 taskset -c 0 ./client -s 10.0.1.5
- sudo perf record -a -g -C 1 nice -n -20 taskset -c 1 ./server 
