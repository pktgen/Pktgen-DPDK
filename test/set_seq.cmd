set all seq_cnt 2
# seq <seq#> <portlist> dst-Mac src-Mac dst-IP src-IP/netmask sport dport ipv4|ipv6|vlan udp|tcp|icmp vid pktsize teid
seq 0 all 0000:4455:6677 0000:1234:5678 10.11.0.1 10.10.0.1/16 5 6 ipv4 udp 1 128 0
seq 1 all 0000:4455:6677 0000:1234:5678 10.11.0.1 10.10.0.1/16 5 6 ipv4 udp 1 128 3

# seq <seq#> <portlist> cos <cos> tos <tos>
seq 0, all cos 4 tos 5
seq 1, all cos 7 tos 6
