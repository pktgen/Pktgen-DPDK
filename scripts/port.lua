-- Port information Demo script
--
-- SPDX-License-Identifier: BSD-3-Clause

package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";

stats = pktgen.portStats("0-3", "rate")
--prints("stats", stats)

printf("%-4s %16s %16s\n", "Port", "PktsRx", "PktsTx")
for n=0,(stats.n - 1) do
    printf("%2d   %16d %16d\n", n, stats[n].pkts_rx, stats[n].pkts_tx)
end
