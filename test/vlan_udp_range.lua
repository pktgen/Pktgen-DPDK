package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"
-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--

local function doWait(port, waitTime)
    local idx;

	pktgen.delay(1000);

	if ( waitTime == 0 ) then
		return;
	end
	waitTime = waitTime - 1;

    -- Try to wait for the total number of packets to be sent.
    local idx = 0;
    while( idx < waitTime ) do

        idx = idx + 1;

        local sending = pktgen.isSending(port);
        if ( sending[tonumber(port)] == "n" ) then
            break;
        end
        pktgen.delay(1000);
    end

end


pktgen.reset("all");

-- 'set' commands for a number of per port values
pktgen.set("all", "count", 100);
pktgen.set("all", "rate", 1);
pktgen.set("all", "size", 64);
pktgen.set("all", "burst", 128);

pktgen.vlanid("all", 55);
pktgen.set_type("all", "ipv4");
pktgen.set_proto("all", "udp");

pktgen.clear("all");
pktgen.cls();

pktgen.pause("Do range commands\n", 1000);
pktgen.page("range");

pktgen.dst_mac("all", "start", "0011:2233:4455");
pktgen.src_mac("all", "start", "0033:2233:4455");

--pktgen.delay(1000);
pktgen.dst_ip("all", "start", "10.10.10.1");
pktgen.dst_ip("all", "inc", "0.0.0.1");
pktgen.dst_ip("all", "min", "10.10.10.1");
pktgen.dst_ip("all", "max", "10.10.10.64");

--pktgen.delay(1000);
pktgen.src_ip("all", "start", "11.11.11.1");
pktgen.src_ip("all", "inc", "0.0.0.1");
pktgen.src_ip("all", "min", "11.11.11.1");
pktgen.src_ip("all", "max", "11.11.11.64");

pktgen.set_proto("all", "udp");

--pktgen.delay(1000);
pktgen.dst_port("all", "start", 2152);
pktgen.dst_port("all", "inc", 0);
pktgen.dst_port("all", "min", 2152);
pktgen.dst_port("all", "max", 2152);

pktgen.delay(1000);
pktgen.src_port("all", "start", 1000);
pktgen.src_port("all", "inc", 1);
pktgen.src_port("all", "min", 1000);
pktgen.src_port("all", "max", 9999);

--pktgen.delay(1000);
pktgen.vlan_id("all", "start", 100);
pktgen.vlan_id("all", "inc", 1);
pktgen.vlan_id("all", "min", 100);
pktgen.vlan_id("all", "max", 4094);

--pktgen.delay(1000);
pktgen.pkt_size("all", "start", 128);
pktgen.pkt_size("all", "inc", 2);
pktgen.pkt_size("all", "min", 64);
pktgen.pkt_size("all", "max", 1518);

pktgen.set_range("all", "on");
pktgen.vlan("all", "on");

pktgen.pause("Wait a second, then go back to main page\n", 1000);

pktgen.page("0");

pktgen.pause("About to do range\n", 2000);

pktgen.start("all");
printf("Waiting for TX to run\n");
doWait("0", 10);
printf("Done\n");

printf("Port Count %d\n", pktgen.portCount());
printf("Total port Count %d\n", pktgen.totalPorts());
