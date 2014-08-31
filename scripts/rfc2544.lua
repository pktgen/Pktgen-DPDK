-- RFC-2544 throughput testing.
--
package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";

local pktSizes = { 64, 128, 256, 512, 1024, 1280, 1518 };
local delayTime = 10;		-- Time in seconds to delay.
local sendport = "0";
local recvport = "2";
local dstip = "10.10.0.100";
local srcip = "10.10.0.101";
local netmask = "/24";

pktgen.set_ipaddr(sendport, "dst", dstip);
pktgen.set_ipaddr(sendport, "src", srcip..netmask);

pktgen.set_ipaddr(recvport, "dst", srcip);
pktgen.set_ipaddr(recvport, "src", dstip..netmask);

pktgen.set_proto(sendport..","..recvport, "udp");

local function doTest(size, rate, limit)
	local endStats, diff, limit, prev, flag;

	printf("*** Test packet size %d at rate %d%%\n", size, rate);
	
	pktgen.set(sendport, "rate", rate);
	pktgen.set(sendport, "size", size);

	pktgen.clr();

	pktgen.start(sendport);
	sleep(delayTime);
	pktgen.stop(sendport);

	endStats = pktgen.portStats(sendport..","..recvport, "port");

	diff = endStats[2].ipackets - endStats[0].opackets;
	printf("%3d Total sent %d recv %d, delta %d\n", cnt, endStats[0].opackets, endStats[2].ipackets, diff);
	
	flag = false;
	if ( diff ~= 0 ) then
		prev = rate;
		if ( diff < 0 ) then
			rate = rate - (prev/2);
		else
			rate = rate + (prev/2);
		end
		flag = doTest(size, rate, limit);
	end

	return flag;
end

function main()
	local rate, prev, flag, cnt;

	for _,size in pairs(pktSizes) do
		rate = 100;
		cnt = 1;
		flag = true;
		
		while( (flag == true) and (cnt < 10 ) ) do
			flag = doTest(size, rate, 0, rate);
			cnt = cnt + 1;
		end
	end
end

main();
