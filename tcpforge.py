from scapy.all import IP, TCP, Raw, send

dst_ip = "192.168.1.1"
dst_port = 80
payload = b"Hello, server!"

packet = IP(dst=dst_ip) / \
    TCP(dport=dst_port, flags="PA", options=None) / \
    Raw(load=payload)

send(packet)