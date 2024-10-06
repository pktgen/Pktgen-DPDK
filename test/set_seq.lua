package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"
-- Lua uses '--' as comment to end of line read the
-- manual for more comment options.
require "Pktgen"
local seq_table = {			-- entries can be in any order
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["ip_dst_addr"] = "10.12.0.1",
    ["ip_src_addr"] = "10.12.0.1/16",	-- the 16 is the size of the mask value
    ["sport"] = 9,			-- Standard port numbers
    ["dport"] = 10,			-- Standard port numbers
    ["ethType"] = "ipv4",	-- ipv4|ipv6|vlan
    ["ipProto"] = "udp",	-- udp|tcp|icmp
    ["vlanid"] = 1,			-- 1 - 4095
    ["pktSize"] = 128,		-- 64 - 1518
    ["teid"] = 3,
    ["cos"] = 5,
    ["tos"] = 6,
    ["tcp_flags"] = "fin,ack,cwr"
  };
-- seqTable( seq#, portlist, table );
pktgen.seqTable(0, "all", seq_table );
pktgen.set("all", "seq_cnt", 4);
