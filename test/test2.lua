package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

pktgen.screen("off");

pktgen.seqTable(pktgen.info.startSeqIdx, "all", {
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["ip_dst_addr"] = "10.11.0.1",
    ["ip_src_addr"] = "10.11.0.1/16",
    ["ethType"] = "ipv4",
    ["ipProto"] = "udp",
    ["tlen"] = 42,
    ["sport"] = 9,
    ["dport"] = 10,
    ["pktSize"] = 128,
    ["vlanid"] = 40
  });

prints("seqTable", pktgen.decompile(pktgen.info.startSeqIdx, "all"));

pktgen.compile(pktgen.info.startExtraIdx, "all", {
    ["eth_dst_addr"] = "0022:4455:6677",
    ["eth_src_addr"] = "0022:1234:5678",
    ["ip_dst_addr"] = "10.12.0.1",
    ["ip_src_addr"] = "10.12.0.1/16",
    ["ethType"] = "ipv4",
    ["ipProto"] = "tcp",
    ["tlen"] = 42,
    ["sport"] = 9,
    ["dport"] = 10,
    ["pktSize"] = 128,
    ["vlanid"] = 40
  });

prints("compile", pktgen.decompile(pktgen.info.startExtraIdx, "all"));

prints("pktgen", pktgen);
