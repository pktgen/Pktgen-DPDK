-- Latency Demo script
--
-- SPDX-License-Identifier: BSD-3-Clause

package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";

local port = 0;
local sleeptime = 10

pktgen.stop(tostring(port));

printf("Setup port %d for latency packets\n", port);
pktgen.clr();
pktgen.delay(100);

pktgen.latency(tostring(port), "enable");

pktgen.start(tostring(port));
printf("Sleep for %d seconds\n", sleeptime);
pktgen.sleep(sleeptime);
printf("Sleep is done\n");

pktgen.stop(tostring(port));
pktgen.latency(tostring(port), "disable");

printf("Sendport Type: %s\n", type(port));
prints("SendPort", pktgen.pktStats(tostring(port)));
local port_stats = pktgen.pktStats(tostring(port));

printf("Number of latency packets on port %d : %d\n",
    port, port_stats[tonumber(port)].latency.num_pkts);
