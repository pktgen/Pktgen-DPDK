package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"
-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--

prints("portStats", pktgen.portStats("all", "port"));
prints("portStats", pktgen.portStats('2', 'port'));
