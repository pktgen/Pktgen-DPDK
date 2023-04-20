-- Latency Demo script
--
-- SPDX-License-Identifier: BSD-3-Clause

package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";

local port = 2;
local sleeptime = 10

pktgen.stop(port);

pktgen.latency(port, "rate", 1000) -- 1000us
pktgen.latency(port, "entropy", 16) -- adjust sport (sport + (index % N))

printf("Setup port %d for latency packets\n", port);
pktgen.clr();
pktgen.delay(100);

pktgen.latency(port, "enable");

pktgen.start(port);
printf("Sleep for %d seconds\n", sleeptime);
pktgen.sleep(sleeptime);
printf("Sleep is done\n");

pktgen.stop(port);
pktgen.latency(port, "disable");

printf("Sendport Type: %s\n", type(port));
prints("SendPort", pktgen.pktStats(port));
local port_stats = pktgen.pktStats(port);

printf("Number of latency packets on port %d : %d\n",
    port, port_stats[port].latency.num_pkts);
