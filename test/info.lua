package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"

-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--
function info()
	printf("Lua Version      : %s\n", pktgen.info.Lua_Version);

	printf("Pktgen Version   : %s\n",
		pktgen.info.Pktgen_Version);
	printf("Pktgen Copyright : %s\n",
		pktgen.info.Pktgen_Copyright);

	prints("pktgen.info",
		pktgen.info);

	printf("Port Count %d\n",
		pktgen.portCount());
	printf("Total port Count %d\n",
		pktgen.totalPorts());
end

info()
