-- Lua uses '--' as comment to end of line read the
-- manual for more comment options.
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
    ["pktSize"] = 128		-- 64 - 1518
  };
-- seqTable( seq#, portlist, table );
pktgen.seqTable(0, "all", seq_table );
pktgen.set("all", "seqCnt", 1);

-- Set the first two ports with know values, must set seqCnt to 0 to use.
--  001b:218e:b760    001b:218e:b1e8
pktgen.set_mac("0", "001b:218e:b760");
pktgen.set_mac("1", "001b:218e:b1e8");

pktgen.set_ipaddr("0", "dst", "20.10.2.2");
pktgen.set_ipaddr("0", "src", "20.10.1.2/24");

pktgen.set_ipaddr("1", "dst", "20.10.1.2");
pktgen.set_ipaddr("1", "src", "20.10.2.2/24");

pktgen.set_proto("all", "udp");
pktgen.set_type("all", "ipv4");

pktgen.set("all", "size", 64);
pktgen.set("all", "sport", 5678);
pktgen.set("all", "dport", 1234);

pktgen.set("all", "count", 0);
pktgen.set("all", "rate", 100);

pktgen.vlan_id("all", 5);
