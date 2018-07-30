package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"
--
-- Set up the sequence data for the port.
pktgen.set('0', 'seq_cnt', 8);

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
  ['ip_src_addr'] = '192.168.0.1/24',
  ['sport'] = 12340,
  ['dport'] = 56780,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[1] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:b9',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:79',
  ['ip_dst_addr'] = '192.168.2.1',
  ['ip_src_addr'] = '192.168.0.2/24',
  ['sport'] = 12341,
  ['dport'] = 56781,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[2] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:ba',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:7a',
  ['ip_dst_addr'] = '192.168.3.1',
  ['ip_src_addr'] = '192.168.0.3/24',
  ['sport'] = 12342,
  ['dport'] = 56782,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[3] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:bb',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:7b',
  ['ip_dst_addr'] = '192.168.4.1',
  ['ip_src_addr'] = '192.168.0.4/24',
  ['sport'] = 12343,
  ['dport'] = 56783,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[4] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:bc',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:7c',
  ['ip_dst_addr'] = '192.168.5.1',
  ['ip_src_addr'] = '192.168.0.5/24',
  ['sport'] = 12344,
  ['dport'] = 56784,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[5] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:bd',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:7d',
  ['ip_dst_addr'] = '192.168.6.1',
  ['ip_src_addr'] = '192.168.0.6/24',
  ['sport'] = 12345,
  ['dport'] = 56785,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[6] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:be',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:7e',
  ['ip_dst_addr'] = '192.168.7.1',
  ['ip_src_addr'] = '192.168.0.7/24',
  ['sport'] = 12346,
  ['dport'] = 56786,
  ['ethType'] = 'ipv4',
  ['ipProto'] = 'tcp',
  ['vlanid'] = 1,
  ['pktSize'] = 60,
  ['gtpu_teid'] = 0
}
seq_table[7] = {
  ['eth_dst_addr'] = '3c:fd:fe:9e:2c:bf',
  ['eth_src_addr'] = '3c:fd:fe:9e:29:7f',
  ['ip_dst_addr'] = '192.168.8.1',
  ['ip_src_addr'] = '192.168.0.8/24',
  ['sport'] = 12347,
  ['dport'] = 56787,
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
pktgen.seqTable(4, '0', seq_table[4]);
pktgen.seqTable(5, '0', seq_table[5]);
pktgen.seqTable(6, '0', seq_table[6]);
pktgen.seqTable(7, '0', seq_table[7]);
