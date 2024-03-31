package.path = package.path .. ";?.lua;test/?.lua;"
require("Pktgen")

pktgen.start("0");
TEST_DURATION = 10
pktgen.delay(TEST_DURATION * 1000);
pktgen.stop("0");
prints("portStats", pktgen.portStats('0', 'port'));
