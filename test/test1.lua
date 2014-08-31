package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--
pktgen.screen("off");

seqTable = {
  [1] = {
    ["pktSize"] = 128,
    ["ip_dst_addr"] = "10.12.0.1",
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["dport"] = 10,
    ["sport"] = 9,
    ["tlen"] = 42,
    ["ip_src_addr"] = "10.12.0.1/16",
    ["vlanid"] = 40,
    ["ipProto"] = "udp",
    ["ethType"] = "ipv4",
  },
  [2] = {
    ["pktSize"] = 128,
    ["ip_dst_addr"] = "10.12.0.1",
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["dport"] = 10,
    ["sport"] = 9,
    ["tlen"] = 42,
    ["ip_src_addr"] = "10.12.0.1/16",
    ["vlanid"] = 40,
    ["ipProto"] = "udp",
    ["ethType"] = "ipv4",
  },
  [3] = {
    ["pktSize"] = 128,
    ["ip_dst_addr"] = "10.12.0.1",
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["dport"] = 10,
    ["sport"] = 9,
    ["tlen"] = 42,
    ["ip_src_addr"] = "10.12.0.1/16",
    ["vlanid"] = 40,
    ["ipProto"] = "udp",
    ["ethType"] = "ipv4",
  },
  [0] = {
    ["pktSize"] = 128,
    ["ip_dst_addr"] = "10.12.0.1",
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["dport"] = 10,
    ["sport"] = 9,
    ["tlen"] = 42,
    ["ip_src_addr"] = "10.12.0.1/16",
    ["vlanid"] = 40,
    ["ipProto"] = "udp",
    ["ethType"] = "ipv4",
  },
  ["n"] = 4,
}

prints("seqTable", seqTable);
