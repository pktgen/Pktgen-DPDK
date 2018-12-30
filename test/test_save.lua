--
-- Pktgen - Ver: 3.0.07 (DPDK 16.07.0-rc3)
-- Copyright (c) <2010-2019>, Intel Corporation. All rights reserved., Powered by DPDK

-- Command line arguments: (DPDK args are defaults)
-- ./app/app/x86_64-native-linuxapp-gcc/app/pktgen -c 3fffe -n 3 -m 512 --proc-type primary -- -T -P -m [2-5:6-9].0 -m [10-13:14-17].1 -f themes/black-yellow.theme

-- #######################################################################
-- Pktgen Configuration script information:
--   GUI socket is Not Enabled
--   Flags 00000804
--   Number of ports: 2
--   Number ports per page: 4
--   Number descriptors: RX 512 TX: 512
--   Promiscuous mode is Enabled

package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"
--#######################################################################
-- Global configuration:
-- geometry 132x44
pktgen.mac_from_arp("disable");

-- ######################### Port  0 ##################################
--
-- Port:  0, Burst: 32, Rate:100%, Flags:00020020, TX Count:Forever
--           SeqCnt:4, Prime:1 VLAN ID:0001, Link: <UP-40000-FD>
--
-- Set up the primary port information:
pktgen.set('0', 'count', 0);
pktgen.set('0', 'size', 64);
pktgen.set('0', 'rate', 100);
pktgen.set('0', 'burst', 32);
pktgen.set('0', 'sport', 1234);
pktgen.set('0', 'dport', 5678);
pktgen.set('0', 'prime', 1);
pktgen.set_type('0', 'ipv4');
pktgen.set_proto('0', 'tcp');
pktgen.set_ipaddr('0', 'dst', '192.168.1.1');
pktgen.set_ipaddr('0', 'src','192.168.0.1/24');
pktgen.set_mac('0', '3c:fd:fe:9e:2c:b8');
pktgen.vlanid('0', 1);

pktgen.pattern('0', 'abc');

pktgen.jitter('0', 50);
pktgen.mpls('0', 'disable');
pktgen.mpls_entry('0', '0');
pktgen.qinq('0', 'disable');
pktgen.qinqids('0', 0, 0);
pktgen.gre('0', 'disable');
pktgen.gre_eth('0', 'disable');
pktgen.gre_key('0', 0);
--
-- Port flag values:
pktgen.icmp_echo('0', 'disable');
pktgen.pcap('0', 'disable');
pktgen.set_range('0', 'disable');
pktgen.latency('0', 'disable');
pktgen.process('0', 'disable');
pktgen.capture('0', 'disable');
pktgen.rxtap('0', 'disable');
pktgen.txtap('0', 'disable');
pktgen.vlan('0', 'disable');

--
-- Range packet information:
pktgen.src_mac('0', 'start', '3c:fd:fe:9e:29:78');
pktgen.src_mac('0', 'min', '00:00:00:00:00:00');
pktgen.src_mac('0', 'max', '00:00:00:00:00:00');
pktgen.src_mac('0', 'inc', '00:00:00:00:00:00');
pktgen.dst_mac('0', 'start', '3c:fd:fe:9e:2c:b8');
pktgen.dst_mac('0', 'min', '00:00:00:00:00:00');
pktgen.dst_mac('0', 'max', '00:00:00:00:00:00');
pktgen.dst_mac('0', 'inc', '00:00:00:00:00:00');

pktgen.src_ip('0', 'start', '192.168.0.1');
pktgen.src_ip('0', 'min', '192.168.0.1');
pktgen.src_ip('0', 'max', '192.168.0.254');
pktgen.src_ip(';0', 'inc', '0.0.0.0');

pktgen.dst_ip('0', 'start', '192.168.1.1');
pktgen.dst_ip('0', 'min', '192.168.1.1');
pktgen.dst_ip('0', 'max', '192.168.1.254');
pktgen.dst_ip('0', 'inc', '0.0.0.1');

pktgen.ip_proto('0', 'tcp');

pktgen.src_port('0', 'start', 0);
pktgen.src_port('0', 'min', 0);
pktgen.src_port('0', 'max', 254);
pktgen.src_port('0', 'inc', 1);

pktgen.dst_port('0', 'start', 0);
pktgen.dst_port('0', 'min', 0);
pktgen.dst_port('0', 'max', 254);
pktgen.dst_port('0', 'inc', 1);

pktgen.vlan_id('0', 'start', 1);
pktgen.vlan_id('0', 'min', 1);
pktgen.vlan_id('0', 'max', 4095);
pktgen.vlan_id('0', 'inc', 0);

pktgen.pkt_size('0', 'start', 64);
pktgen.pkt_size('0', 'min', 64);
pktgen.pkt_size('0', 'max', 1518);
pktgen.pkt_size('0', 'inc', 0);

--
-- Set up the sequence data for the port.
pktgen.set('0', 'seq_cnt', 4);

-- (seqnum, port, dst_mac, src_mac, ip_dst, ip_src, sport, dport, ethType, proto, vlanid, pktSize, gtpu_teid)
-- pktgen.seq(0, '0', '3c:fd:fe:9e:2c:b8' '3c:fd:fe:9e:29:78', '192.168.1.1', '192.168.0.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
-- pktgen.seq(1, '0', '3c:fd:fe:9e:2c:b8' '3c:fd:fe:9e:29:78', '192.168.1.1', '192.168.0.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
-- pktgen.seq(2, '0', '3c:fd:fe:9e:2c:b8' '3c:fd:fe:9e:29:78', '192.168.1.1', '192.168.0.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
-- pktgen.seq(3, '0', '3c:fd:fe:9e:2c:b8' '3c:fd:fe:9e:29:78', '192.168.1.1', '192.168.0.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
local seq_table = {}
seq_table[0] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:b8',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:78',
  ['ip_dst_addr'] = '192.168.1.1',
  ['ip_src_addr'] = '192.168.0.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[1] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:b8',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:78',
  ['ip_dst_addr'] = '192.168.1.1',
  ['ip_src_addr'] = '192.168.0.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[2] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:b8',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:78',
  ['ip_dst_addr'] = '192.168.1.1',
  ['ip_src_addr'] = '192.168.0.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[3] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:b8',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:78',
  ['ip_dst_addr'] = '192.168.1.1',
  ['ip_src_addr'] = '192.168.0.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
pktgen.seqTable(0, '0', seq_table[0]);
pktgen.seqTable(1, '0', seq_table[1]);
pktgen.seqTable(2, '0', seq_table[2]);
pktgen.seqTable(3, '0', seq_table[3]);


-- Rnd bitfeilds
pktgen.rnd('0', 0, 100, 'XXX...111.000...111.XXX.........');
pktgen.rnd('0', 1, 100, 'X11...111.000...111.XXX.........');
-- ######################### Port  1 ##################################
--
-- Port:  1, Burst: 32, Rate:100%, Flags:00020020, TX Count:Forever
--           SeqCnt:4, Prime:1 VLAN ID:0001, Link: <UP-40000-FD>
--
-- Set up the primary port information:
pktgen.set('1', 'count', 0);
pktgen.set('1', 'size', 64);
pktgen.set('1', 'rate', 100);
pktgen.set('1', 'burst', 32);
pktgen.set('1', 'sport', 1234);
pktgen.set('1', 'dport', 5678);
pktgen.set('1', 'prime', 1);
pktgen.set_type('1', 'ipv4');
pktgen.set_proto('1', 'tcp');
pktgen.set_ipaddr('1', 'dst', '192.168.0.1');
pktgen.set_ipaddr('1', 'src','192.168.1.1/24');
pktgen.set_mac('1', '3c:fd:fe:9e:29:78');
pktgen.vlanid('1', 1);

pktgen.pattern('1', 'abc');

pktgen.jitter('1', 50);
pktgen.mpls('1', 'disable');
pktgen.mpls_entry('1', '0');
pktgen.qinq('1', 'disable');
pktgen.qinqids('1', 0, 0);
pktgen.gre('1', 'disable');
pktgen.gre_eth('1', 'disable');
pktgen.gre_key('1', 0);
--
-- Port flag values:
pktgen.icmp_echo('1', 'disable');
pktgen.pcap('1', 'disable');
pktgen.set_range('1', 'disable');
pktgen.latency('1', 'disable');
pktgen.process('1', 'disable');
pktgen.capture('1', 'disable');
pktgen.rxtap('1', 'disable');
pktgen.txtap('1', 'disable');
pktgen.vlan('1', 'disable');

--
-- Range packet information:
pktgen.src_mac('1', 'start', '3c:fd:fe:9e:2c:b8');
pktgen.src_mac('1', 'min', '00:00:00:00:00:00');
pktgen.src_mac('1', 'max', '00:00:00:00:00:00');
pktgen.src_mac('1', 'inc', '00:00:00:00:00:00');
pktgen.dst_mac('1', 'start', '3c:fd:fe:9e:29:78');
pktgen.dst_mac('1', 'min', '00:00:00:00:00:00');
pktgen.dst_mac('1', 'max', '00:00:00:00:00:00');
pktgen.dst_mac('1', 'inc', '00:00:00:00:00:00');

pktgen.src_ip('1', 'start', '192.168.1.1');
pktgen.src_ip('1', 'min', '192.168.1.1');
pktgen.src_ip('1', 'max', '192.168.1.254');
pktgen.src_ip(';1', 'inc', '0.0.0.0');

pktgen.dst_ip('1', 'start', '192.168.2.1');
pktgen.dst_ip('1', 'min', '192.168.2.1');
pktgen.dst_ip('1', 'max', '192.168.2.254');
pktgen.dst_ip('1', 'inc', '0.0.0.1');

pktgen.ip_proto('1', 'tcp');

pktgen.src_port('1', 'start', 256);
pktgen.src_port('1', 'min', 256);
pktgen.src_port('1', 'max', 510);
pktgen.src_port('1', 'inc', 1);

pktgen.dst_port('1', 'start', 256);
pktgen.dst_port('1', 'min', 256);
pktgen.dst_port('1', 'max', 510);
pktgen.dst_port('1', 'inc', 1);

pktgen.vlan_id('1', 'start', 1);
pktgen.vlan_id('1', 'min', 1);
pktgen.vlan_id('1', 'max', 4095);
pktgen.vlan_id('1', 'inc', 0);

pktgen.pkt_size('1', 'start', 64);
pktgen.pkt_size('1', 'min', 64);
pktgen.pkt_size('1', 'max', 1518);
pktgen.pkt_size('1', 'inc', 0);

--
-- Set up the sequence data for the port.
pktgen.set('1', 'seq_cnt', 4);

-- (seqnum, port, dst_mac, src_mac, ip_dst, ip_src, sport, dport, ethType, proto, vlanid, pktSize, gtpu_teid)
-- pktgen.seq(0, '1', '3c:fd:fe:9e:29:78' '3c:fd:fe:9e:2c:b8', '192.168.0.1', '192.168.1.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
-- pktgen.seq(1, '1', '3c:fd:fe:9e:29:78' '3c:fd:fe:9e:2c:b8', '192.168.0.1', '192.168.1.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
-- pktgen.seq(2, '1', '3c:fd:fe:9e:29:78' '3c:fd:fe:9e:2c:b8', '192.168.0.1', '192.168.1.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
-- pktgen.seq(3, '1', '3c:fd:fe:9e:29:78' '3c:fd:fe:9e:2c:b8', '192.168.0.1', '192.168.1.1/24', 1234, 5678, 'ipv4', 'tcp', 1, 64, 0);
local seq_table = {}
seq_table[0] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:29:78',
  ['eth_src_addr'] = '3c:fd:fe:9e:2c:b8',
  ['ip_dst_addr'] = '192.168.0.1',
  ['ip_src_addr'] = '192.168.1.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[1] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:29:78',
  ['eth_src_addr'] = '3c:fd:fe:9e:2c:b8',
  ['ip_dst_addr'] = '192.168.0.1',
  ['ip_src_addr'] = '192.168.1.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[2] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:29:78',
  ['eth_src_addr'] = '3c:fd:fe:9e:2c:b8',
  ['ip_dst_addr'] = '192.168.0.1',
  ['ip_src_addr'] = '192.168.1.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[3] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:29:78',
  ['eth_src_addr'] = '3c:fd:fe:9e:2c:b8',
  ['ip_dst_addr'] = '192.168.0.1',
  ['ip_src_addr'] = '192.168.1.1',
  ['sport'] = 1234,
  ['dport'] = 5678,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
pktgen.seqTable(0, '1', seq_table[0]);
pktgen.seqTable(1, '1', seq_table[1]);
pktgen.seqTable(2, '1', seq_table[2]);
pktgen.seqTable(3, '1', seq_table[3]);


-- Rnd bitfeilds
pktgen.rnd('1', 1, 100, 'X11...111.000...111.XXX.........');
-- ################################ Done #################################
