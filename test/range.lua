pktgen.page("range");

pktgen.range.dst_mac("all", "start", "000d:49ef:016d");
pktgen.range.src_mac("all", "start", "000d:49ef:0163");

pktgen.delay(1000);
pktgen.range.dst_ip("all", "start", "10.10.10.1");
pktgen.range.dst_ip("all", "inc", "0.0.0.1");
pktgen.range.dst_ip("all", "min", "10.10.10.1");
pktgen.range.dst_ip("all", "max", "10.10.10.4");

pktgen.delay(1000);
pktgen.range.src_ip("all", "start", "20.20.20.1");
pktgen.range.src_ip("all", "inc", "0.0.0.1");
pktgen.range.src_ip("all", "min", "20.20.20.1");
pktgen.range.src_ip("all", "max", "20.20.20.8");

pktgen.delay(1000);
pktgen.range.ttl("all", "start", 64);
pktgen.range.ttl("all", "inc", 0);
pktgen.range.ttl("all", "min", 64);
pktgen.range.ttl("all", "max", 64);

