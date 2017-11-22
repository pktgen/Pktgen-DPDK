#!/bin/bash

# generate-flows.sh
# quick and dirty script to generate flows based on the below settings.
# Right now assumes just two ports.
#
# Contributed by gkenaley

#TODO --
# Could be improved to do multi-port and parameterized.  Maybe prompt
# for the information 
#

#For Port 0/1
DEST_MAC=fa:16:3e:b9:de:22
SRC_MAC=fa:16:3e:92:9c:03
SRC_IP=192.168.30.4
DEST_IP=192.168.30.50
VLAN=1
IPMODE=ipv4
PROT=udp
PKTSIZE=512
FLOWS=10

#IP Settings below may not be required unless using tcp/ip
if [[ "${PROT,,}" != "udp" ]]; then
	echo "set ip dst 0 $SRC_IP"
	echo "set ip src 0 $DEST_IP/24"
	echo "set ip dst 1 $DEST_IP"
	echo "set ip src 1 $SRC_IP/24"
fi

echo "set 0,1 size $PKTSIZE"

echo "set 0,1 rate 1"

#port 0
for flow in `seq 0 $(($FLOWS - 1))`
do
		 #
		 #seq <seq#>   <portlist>  dst-Mac  src-Mac  dst-IP    src-IP/nm    sport   dport   ipv4|ipv6   udp|tcp|icmp   vid   pktsize
		 #
	echo "seq $flow 0 $DEST_MAC $SRC_MAC $DEST_IP $SRC_IP/24 $((1000 + $flow)) $((2000 + $flow)) $IPMODE $PROT $VLAN $PKTSIZE"
	#todo maybe add changing something other than port
	
done

#port 1
for flow in `seq 0 $(($FLOWS - 1))`
do
	echo "seq $flow 1  $SRC_MAC $DEST_MAC $SRC_IP $DEST_IP/24 $((1000+$flow)) $((2000+$flow)) $IPMODE $PROT $VLAN $PKTSIZE"
	#todo maybe add changing something other than port
	
done

echo "set 0,1 seqCnt $FLOWS"
