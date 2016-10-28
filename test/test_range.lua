package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

pktgen.page("range");

pktgen.range.dst_mac("0", "start", "0011:2233:4455");
pktgen.range.src_mac("0", "start", "0033:2233:4455");

pktgen.range.dst_ip("0", "start", "10.10.10.1");
pktgen.range.dst_ip("0", "inc", "0.0.0.0");
pktgen.range.dst_ip("0", "min", "10.10.10.1");
pktgen.range.dst_ip("0", "max", "10.10.10.1");

pktgen.range.src_ip("0", "start", "11.11.11.1");
pktgen.range.src_ip("0", "inc", "0.0.0.0");
pktgen.range.src_ip("0", "min", "11.11.11.1");
pktgen.range.src_ip("0", "max", "11.11.11.1");

pktgen.set_proto("0", "udp");

pktgen.range.dst_port("0", "start", 2000);
pktgen.range.dst_port("0", "inc", 1);
pktgen.range.dst_port("0", "min", 2000);
pktgen.range.dst_port("0", "max", 4000);

pktgen.range.src_port("0", "start", 5000);
pktgen.range.src_port("0", "inc", 1);
pktgen.range.src_port("0", "min", 5000);
pktgen.range.src_port("0", "max", 7000);

pktgen.range.pkt_size("0", "start", 64);
pktgen.range.pkt_size("0", "inc", 0);
pktgen.range.pkt_size("0", "min", 64);
pktgen.range.pkt_size("0", "max", 64);

-- Set up second port
pktgen.dst_mac("1", "start", "0011:2233:4455");
pktgen.src_mac("1", "start", "0033:2233:4455");

pktgen.dst_ip("1", "start", "10.10.10.1");
pktgen.dst_ip("1", "inc", "0.0.0.0");
pktgen.dst_ip("1", "min", "10.10.10.1");
pktgen.dst_ip("1", "max", "10.10.10.1");

pktgen.src_ip("1", "start", "11.11.11.1");
pktgen.src_ip("1", "inc", "0.0.0.0");
pktgen.src_ip("1", "min", "11.11.11.1");
pktgen.src_ip("1", "max", "11.11.11.1");

pktgen.set_proto("all", "udp");

pktgen.dst_port("1", "start", 2000);
pktgen.dst_port("1", "inc", 1);
pktgen.dst_port("1", "min", 2000);
pktgen.dst_port("1", "max", 4000);

pktgen.src_port("1", "start", 5000);
pktgen.src_port("1", "inc", 1);
pktgen.src_port("1", "min", 5000);
pktgen.src_port("1", "max", 7000);

pktgen.pkt_size("1", "start", 64);
pktgen.pkt_size("1", "inc", 0);
pktgen.pkt_size("1", "min", 64);
pktgen.pkt_size("1", "max", 64);

pktgen.set_range("all", "on");
