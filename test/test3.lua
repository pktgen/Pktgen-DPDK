package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--

prints("linkState", pktgen.linkState("all"));
prints("isSending", pktgen.isSending("all"));
prints("portSizes", pktgen.portSizes("all"));
prints("pktStats", pktgen.pktStats("all"));
prints("portRates", pktgen.portStats("all", "rate"));
prints("portStats", pktgen.portStats("all", "port"));
