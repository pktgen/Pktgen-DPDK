#!/usr/bin/env python3
import argparse
from scapy.all import *


parser = argparse.ArgumentParser()
parser.add_argument('-n', '--nflows', required=True)
parser.add_argument('-o', '--output', required=True)
args = parser.parse_args()

dst_mac = "50:6b:4b:93:2a:3c"
src_mac = "52:54:00:ab:ec:e7"
src_ip = "10.0.0.101"
dst_ip = "142.0.0.1"
dst_port = 8080
payload = "testdata"
src_srh = "fc00:aa00::"
dst_srh = "fc00:4101::"
sidlist = ['fc00:4101::']
pcap_file = args.output

packets = []
for src_port in range(1024, 1024 + int(args.nflows)):
    tmp = Ether(src=src_mac, dst=dst_mac)
    tmp = tmp / IPv6(src=src_srh, dst=dst_srh, nh=43, fl=src_port)
    tmp = tmp / IPv6ExtHdrSegmentRouting(segleft=len(sidlist)-1, lastentry=len(sidlist)-1, addresses=sidlist)
    tmp = tmp / IP(src=src_ip, dst=dst_ip)
    tmp = tmp / UDP(sport=src_port, dport=dst_port)
    tmp = tmp / payload
    packets.append(tmp)
wrpcap(pcap_file, packets)
