-- RFC-2544 throughput testing.
--
-- SPDX-License-Identifier: BSD-3-Clause

package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";

local pktSizes		= { 64, 128, 256, 512, 1024, 1280, 1518 };
local firstDelay	= 3;
local delayTime		= 30;		-- Time in seconds to wait for tx to stop
local pauseTime		= 1;
local sendport		= "0";
local recvport		= "2";
local dstip			= "10.10.0.100";
local srcip			= "10.10.0.101";
local netmask		= "/24";
local pktCnt		= 4000000;
local foundRate;

pktgen.set_ipaddr(sendport, "dst", dstip);
pktgen.set_ipaddr(sendport, "src", srcip..netmask);

pktgen.set_ipaddr(recvport, "dst", srcip);
pktgen.set_ipaddr(recvport, "src", dstip..netmask);

pktgen.set_proto(sendport..","..recvport, "udp");


local function doWait(port)
	local stat, idx, diff;

	-- Try to wait for the total number of packets to be sent.
	local idx = 0;
	while( idx < (delayTime - firstDelay) ) do
		stat = pktgen.portStats(port, "port")[tonumber(port)];

		diff = stat.ipackets - pktCnt;
		--print(idx.." ipackets "..stat.ipackets.." delta "..diff);

		idx = idx + 1;
		if ( diff == 0 ) then
			break;
		end

		local sending = pktgen.isSending(sendport);
		if ( sending[tonumber(sendport)] == "n" ) then
			break;
		end
		pktgen.delay(pauseTime * 1000);
	end

end

local function testRate(size, rate)
	local stat, diff;

	pktgen.set(sendport, "rate", rate);
	pktgen.set(sendport, "size", size);

	pktgen.clr();
	pktgen.delay(500);

	pktgen.start(sendport);
	pktgen.delay(firstDelay * 1000);

	doWait(recvport);

	pktgen.stop(sendport);

	pktgen.delay(pauseTime * 1000);

	stat = pktgen.portStats(recvport, "port")[tonumber(recvport)];
	diff = stat.ipackets - pktCnt;
	--printf(" delta %10d", diff);

	return diff;
end

local function midpoint(imin, imax)
	return (imin + ((imax - imin) / 2));
end

local function doSearch(size, minRate, maxRate)
	local diff, midRate;

	if ( maxRate < minRate ) then
		return 0.0;
	end

	midRate = midpoint(minRate, maxRate);

	printf("    Testing Packet size %4d at %3.0f%% rate", size, midRate);
	printf(" (%3.0f, %3.0f, %3.0f)\n", minRate, midRate, maxRate);
	diff = testRate(size, midRate);

	if ( diff < 0 ) then
		printf("\n");
		return doSearch(size, minRate, midRate);
	elseif ( diff > 0 ) then
		printf("\n");
		return doSearch(size, midRate, maxRate);
	else
		if ( midRate > foundRate ) then
			foundRate = midRate;
		end
		if ( (foundRate == 100.0) or (foundRate == 1.0) ) then
			return foundRate;
		end
		if ( (minRate == midRate) and (midRate == maxRate) ) then
			return foundRate;
		end
		return doSearch(size, midRate, maxRate);
	end
end

function main()
	local size;

	pktgen.clr();

	pktgen.set(sendport, "count", pktCnt);

	print("\nRFC2544 testing... (Not working Completely) ");

	for _,size in pairs(pktSizes) do
		foundRate = 0.0;
		printf("    >>> Max Rate %3.0f%%\n", doSearch(size, 1.0, 100.0));
	end
end

main();
