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

--pktgen.delay(1000);
--pktgen.range.set_type("all", "ipv6");

--pktgen.delay(1000);
--pktgen.range.dst_ip("all", "start", "10a:a0a::");
--pktgen.range.dst_ip("all", "inc", "100::");
--pktgen.range.dst_ip("all", "min", "10a:a0a::");
--pktgen.range.dst_ip("all", "max", "40a:a0a::");

--pktgen.delay(1000);
--pktgen.range.src_ip("all", "start", "114:1414::");
--pktgen.range.src_ip("all", "inc", "94:1414::");
--pktgen.range.src_ip("all", "min", "114:1414::");
--pktgen.range.src_ip("all", "max", "914:1414::");

--pktgen.delay(1000);
--pktgen.range.hop_limits("all", "start", 4);
--pktgen.range.hop_limits("all", "inc", 0);
--pktgen.range.hop_limits("all", "min", 4);
--pktgen.range.hop_limits("all", "max", 4);

--pktgen.delay(1000);
--pktgen.range.traffic_class("all", "start", 32);
--pktgen.range.traffic_class("all", "inc", 0);
--pktgen.range.traffic_class("all", "min", 32);
--pktgen.range.traffic_class("all", "max", 32);

