--
-- PktgenGUI.lua
--

gui = {}

function gui.msg(...)
	str = strfmt("msg=%s\n",strfmt(...));
	io.write(str);
	io.flush();
end

function gui.dumpStats(func, name, portlist)
	stats = func(portlist);
	printf("%s={ ", name);
	st = stats[0];
	for k,v in pairs(st) do 
		printf("%s ", k);
	end
	printf("},");
	for i = 0, (stats.n - 1) do
		st = stats[i];
		printf("%d={ ", i);
		for k,v in pairs(st) do 
			printf("%d ", v);
		end
		if ( i == (stats.n - 1) ) then
			printf("}\n");
		else
			printf("},");
		end
	end
end

function gui.dumpInfo(func, name, portlist)
	stats = func(portlist);
	printf("%s={ ", name);
	st = stats[0];
	for k,v in pairs(st) do 
		printf("%s ", k);
	end
	printf("},");
	for i = 0, (stats.n - 1) do
		st = stats[i];
		printf("%d={ ", i);
		for k,v in pairs(st) do 
			printf("%s ", v);
		end
		if ( i == (stats.n - 1) ) then
			printf("}\n");
		else
			printf("},");
		end
	end
end

function gui.getPktStats(portlist)
	gui.dumpStats(pktgen.pktStats, "pktStats", portlist)
end

function gui.getPortStats(portlist)
	gui.dumpStats(pktgen.portStats, "portStats", portlist)
end

function gui.getRateStats(portlist)
	gui.dumpStats(pktgen.rateStats, "rateStats", portlist)
end

function gui.getPortSizes(portlist)
	gui.dumpStats(pktgen.portSizes, "portSizes", portlist)
end

function gui.getPortInfo(portlist)
	gui.dumpInfo(pktgen.portInfo, "portInfo", portlist)
end

function gui.getLinkState(portlist)
	links = pktgen.linkState(portlist);
	printf("linkState={ ");
	for k,v in pairs(links) do
		if ( k == "n" ) then break; end
		printf("%d=%s ", k, v);
	end
	printf("}\n");
end

function gui.getPortFlags(portlist)
	links = pktgen.portFlags(portlist);
	printf("portFlags={ ");
	for k,v in pairs(links) do
		if ( k == "n" ) then break; end
		printf("%d=%s ", k, v);
	end
	printf("}\n");
end

function gui.startTransmitting(portlist)
	-- gui.msg("=== Start Transmit %s", portlist);
	pktgen.start(portlist);
end

function gui.stopTransmitting(portlist)
	-- gui.msg("=== Stop Transmit %s", portlist);
	pktgen.stop(portlist);
end

function gui.clearAllStats()
	pktgen.clear("all");
end

function gui.noop()
	-- Do nothing
end
