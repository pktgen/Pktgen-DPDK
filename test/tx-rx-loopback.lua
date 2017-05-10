package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"
require "Pktgen";

local pkt_size          = 64;
local duration          = 10000;
local pauseTime         = 1000;

local sendport          = "0";
local recvport          = "1";
local num_tx, num_rx, num_dropped;


-- 'set' commands for a number of per port values
pktgen.set("all", "rate", 100);
pktgen.set("all", "size", pkt_size);
-- send continuous stream of traffic
pktgen.set("all", "count", 0);
-- pktgen.set("all", "burst", 128);
pktgen.set("all", "sport", 0x5678);
pktgen.set("all", "dport", 0x9988);
-- pktgen.set("all", "prime", 3);

pktgen.set_ipaddr("0", "dst", "172.17.25.150");
pktgen.set_ipaddr("0", "src", "172.17.25.15/24");
pktgen.set_ipaddr("1", "dst", "172.17.25.15");
pktgen.set_ipaddr("1", "src", "172.17.25.150/24");
pktgen.set_proto("all", "udp");
-- pktgen.set_type("all", "ipv4");


--pktgen.clr();
pktgen.start("all");
pktgen.delay(duration);
pktgen.stop("all");
pktgen.delay(pauseTime);


statTx = pktgen.portStats(sendport, "port")[tonumber(sendport)];
statRx = pktgen.portStats(recvport, "port")[tonumber(recvport)];
num_tx = statTx.opackets;
num_rx = statRx.ipackets;
num_dropped = num_tx - num_rx;
print("Tx: " .. num_tx .. ". Rx: " .. num_rx .. ". Dropped: " .. num_dropped);

prints("DEBUG portStats", pktgen.portStats("all", "port"));

