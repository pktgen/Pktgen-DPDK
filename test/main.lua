package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--
pktgen.screen("off");
pktgen.pause("Screen off\n", 2000);
pktgen.screen("on");
pktgen.pause("Screen on\n", 2000);
pktgen.screen("off");
pktgen.pause("Screen off\n", 2000);

printf("delay for 1 second\n");
pktgen.delay(1000);
printf("done\n");

-- 'set' commands for a number of per port values
pktgen.set("all", "count", 100);
pktgen.set("all", "rate", 50);
pktgen.set("all", "size", 256);
pktgen.set("all", "burst", 128);
pktgen.set("all", "sport", 0x5678);
pktgen.set("all", "dport", 0x9988);
pktgen.set("all", "prime", 3);
pktgen.set("all", "seqCnt", 3);

pktgen.vlanid("all", 55);

pktgen.screen("on");
pktgen.pause("Screen on\n", 2000);
pktgen.screen("off");

-- sequence command in one line
pktgen.seq(0, "all", "0000:4455:6677", "0000:1234:5678", "10.11.0.1", "10.10.0.1/16", 5, 6, "ipv4", "udp", 1, 128);
prints("seq", pktgen.decompile(0, "all"));

-- sequence command using a table of packet configurations
local seq_table = {
    ["eth_dst_addr"] = "0011:4455:6677",
    ["eth_src_addr"] = "0011:1234:5678",
    ["ip_dst_addr"] = "10.12.0.1",
    ["ip_src_addr"] = "10.12.0.1/16",
    ["sport"] = 9,
    ["dport"] = 10,
    ["ethType"] = "ipv4",
    ["ipProto"] = "udp",
    ["vlanid"] = 1,
    ["pktSize"] = 128
  };
pktgen.seqTable(0, "all", seq_table );

prints("seqTable", pktgen.decompile(0, "all"));

pktgen.ports_per_page(2);
pktgen.icmp_echo("all", "on");
pktgen.send_arp("all", "g");
pktgen.set_mac("0-2", "0000:1122:3344");
pktgen.mac_from_arp("on");
pktgen.set_ipaddr("0", "dst", "10.10.2.2");
pktgen.set_ipaddr("0", "src", "10.10.1.2/24");
pktgen.set_ipaddr("1", "dst", "10.10.2.2");
pktgen.set_ipaddr("1", "src", "10.10.2.2/24");
pktgen.set_proto("all", "udp");
pktgen.set_type("all", "ipv6");
pktgen.ping4("all");
--pktgen.ping6("all");
--pktgen.show("all", "scan");
pktgen.pcap("all", "on");
pktgen.ports_per_page(4);
pktgen.start("all");
pktgen.stop("all");
pktgen.prime("all");
pktgen.delay(1000);

pktgen.screen("on");
pktgen.clear("all");
pktgen.cls();
pktgen.reset("all");

pktgen.pause("Do range commands\n", 1000);
pktgen.page("range");
pktgen.dst_mac("all", "0011:2233:4455");
pktgen.src_mac("all", "0033:2233:4455");

pktgen.delay(1000);
pktgen.dst_ip("all", "start", "10.12.0.1");
pktgen.dst_ip("all", "inc", "0.0.0.2");
pktgen.dst_ip("all", "min", "10.12.0.1");
pktgen.dst_ip("all", "max", "10.12.0.64");

pktgen.delay(1000);
pktgen.src_ip("all", "start", "10.13.0.1");
pktgen.src_ip("all", "inc", "0.0.0.3");
pktgen.src_ip("all", "min", "10.13.0.1");
pktgen.src_ip("all", "max", "10.13.0.64");

pktgen.delay(1000);
pktgen.dst_port("all", "start", 1234);
pktgen.dst_port("all", "inc", 4);
pktgen.dst_port("all", "min", 1234);
pktgen.dst_port("all", "max", 2345);

pktgen.delay(1000);
pktgen.src_port("all", "start", 5678);
pktgen.src_port("all", "inc", 5);
pktgen.src_port("all", "min", 1234);
pktgen.src_port("all", "max", 9999);

pktgen.delay(1000);
pktgen.vlan_id("all", "start", 1);
pktgen.vlan_id("all", "inc", 0);
pktgen.vlan_id("all", "min", 1);
pktgen.vlan_id("all", "max", 4094);

pktgen.delay(1000);
pktgen.pkt_size("all", "start", 128);
pktgen.pkt_size("all", "inc", 2);
pktgen.pkt_size("all", "min", 64);
pktgen.pkt_size("all", "max", 1518);

pktgen.pause("Wait a second, then go back to main page\n", 2000);

pktgen.page("0");
pktgen.pause("About to do range\n", 1000);
pktgen.range("all", "on");

pktgen.port(2);
pktgen.process("all", "on");
pktgen.blink("0", "on");
pktgen.pause("Pause for a while, then turn off screen\n", 4000);
pktgen.screen("off");

printf("Lua Version      : %s\n", pktgen.info.Lua_Version);
printf("Pktgen Version   : %s\n", pktgen.info.Pktgen_Version);
printf("Pktgen Copyright : %s\n", pktgen.info.Pktgen_Copyright);

prints("pktgen.info", pktgen.info);

printf("Port Count %d\n", pktgen.portCount());
printf("Total port Count %d\n", pktgen.totalPorts());

printf("\nDone, Key pressed is (%s)\n", pktgen.continue("\nPress any key: "));
if ( key == "s" ) then
	pktgen.set("all", "seqCnt", 4);
	pktgen.save("foobar.cmd");
	pktgen.continue("Saved foobar.cmd, press key to load that file: ");
	pktgen.load("foobar.cmd");
end
