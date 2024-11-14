package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";
pkt_size = 64;

local dstip     = "10.12.0.1";
local min_dstip = "10.12.0.1";
local max_dstip = "10.12.0.64";
local inc_dstip = "0.0.0.1"

local srcip     = "10.12.0.101";
local min_srcip = "10.12.0.101";
local max_srcip = "10.12.0.101";
local inc_srcip = "0.0.0.0"

local dst_port     = 5678
local inc_dst_port = 0;
local min_dst_port = dst_port
local max_dst_port = dst_port;

local src_port     = 1234
local inc_src_port = 0;
local min_src_port = src_port
local max_src_port = src_port;

printf("\nStarting Experiment!!!\n");
print("Pkt_size is", pkt_size, "\n");

pktgen.set("all", "size", pkt_size)

pktgen.page("range")

pktgen.range.dst_port("all", "start", dst_port);
pktgen.range.dst_port("all", "inc", inc_dst_port);
pktgen.range.dst_port("all", "min", min_dst_port);
pktgen.range.dst_port("all", "max", max_dst_port);

pktgen.range.src_port("all", "start", src_port);
pktgen.range.src_port("all", "inc", inc_src_port);
pktgen.range.src_port("all", "min", min_src_port);
pktgen.range.src_port("all", "max", max_src_port);

pktgen.range.dst_ip("all", "start", dstip);
pktgen.range.dst_ip("all", "inc", inc_dstip);
pktgen.range.dst_ip("all", "min", min_dstip);
pktgen.range.dst_ip("all", "max", max_dstip);

pktgen.range.src_ip("all", "start", srcip);
pktgen.range.src_ip("all", "inc", inc_srcip);
pktgen.range.src_ip("all", "min", srcip);
pktgen.range.src_ip("all", "max", srcip);

pktgen.set_range("0", "on");
