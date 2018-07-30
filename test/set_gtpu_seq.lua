package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"
-- Lua uses '--' as comment to end of line read the
-- manual for more comment options.
require "Pktgen"
local seq_table = {}
seq_table[0] = {            -- entries can be in any order
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0033:1234:5678",
    ["ip_dst_addr"] = "10.11.0.1",
    ["ip_src_addr"] = "10.12.0.1/16",  -- the 16 is the size of the mask value
    ["sport"] = 9000,          -- Standard port numbers
    ["dport"] = 2152,          -- Standard port numbers
    ["ethType"] = "ipv4",  -- ipv4|ipv6|vlan
    ["ipProto"] = "udp",   -- udp|tcp|icmp
    ["vlanid"] = 100,          -- 1 - 4095
    ["pktSize"] = 128,     -- 64 - 1518
    ["gtpu_teid"] = 1000   -- GTPu TEID (Set dport=2152)
  };
seq_table[1] = {            -- entries can be in any order
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0033:1234:5678",
    ["ip_dst_addr"] = "10.11.0.1",
    ["ip_src_addr"] = "10.12.0.1/16",  -- the 16 is the size of the mask value
    ["sport"] = 9000,          -- Standard port numbers
    ["dport"] = 2152,          -- Standard port numbers
    ["ethType"] = "ipv4",  -- ipv4|ipv6|vlan
    ["ipProto"] = "udp",   -- udp|tcp|icmp
    ["vlanid"] = 100,          -- 1 - 4095
    ["pktSize"] = 128,     -- 64 - 1518
    ["gtpu_teid"] = 1000   -- GTPu TEID (Set dport=2152)
  };
seq_table.n = 2;
-- seqTable( seq#, portlist, table );
pktgen.seqTable(0, "all", seq_table[0] );
pktgen.seqTable(1, "all", seq_table[1] );
pktgen.set("all", "seq_cnt", 2);
pktgen.set("all", "seqCnt", 4);
pktgen.page("seq");

