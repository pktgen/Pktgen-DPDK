-- RFC2544 PktGen Throughput Test
-- as defined by https://www.ietf.org/rfc/rfc2544.txt
--  SPDX-License-Identifier: BSD-3-Clause
--
--  Improved 2x100GBE version v1.1 - Contributed by Niclas Edin 2022   
--  Set 4 CPU cores per Rx:Tx function for +200MPPS performance, using Intel Ice Lake-SP platform with C810 NICs.
--  Using ranges for RSS multiq support and setting multiple IP pairs for better DUT load balancing 
--  To avoid 0-loss test related issues, a loss tolerance value is used to get as close to good-put as possible. 
--   
--  For C810 100GbE PPS performance and avoiding Tx degradation, MBUFS need to be increased and burst size lowered to 64 test has shown. 
--  Could also be a factor for other NICs.
--  Also consider setting isolcpu at the kernel cmdline for best DPDK performance as per DPDK-Performance-guide documentation 
--  app/pktgen_constants.h:
--    DEFAULT_PKT_BURST = 64, /* Increasing this number consumes memory very fast */
--    DEFAULT_RX_DESC   = (DEFAULT_PKT_BURST * 16),
--    DEFAULT_TX_DESC   = DEFAULT_RX_DESC * 2,
--    MAX_MBUFS_PER_PORT = (DEFAULT_TX_DESC * 10), /* number of buffers to support per port */
--
--
-- Load from within PktGen CLI. 
--

package.path = package.path ..";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen";
local DEBUG			= true;
local USE_RAMPUP	= true; 	--Rampup traffic flow before starting measurement, less accurate measurement (trial time,drops).
local SHOW_STATUS	= true;
--
-- Logfile for testresult
local logfile 		= "RFC2544_throughput_results.txt";

-- define packet sizes to test
local pkt_sizes	= { 64, 128, 256, 512, 1024, 1280, 1518 };

-- Set max rate platform profile for PktGen to avoid crashing some interfaces during overload (XXV710/C810)
-- Could be mitigated via setting MBUF as top comment describes.
-- Need to be evaluated per pktgen platform setup with a loopback test before testing DUT
local pktgen_limit	= {
  				[64]   = { ["rate"] = 78 },	-- 68% eq 200MPPS on 2x100GbE	
  				[128]  = { ["rate"] = 100 },	
  				[256]  = { ["rate"] = 100 },	
  				[512]  = { ["rate"] = 100 },	
  				[1024] = { ["rate"] = 100 },	
  				[1280] = { ["rate"] = 100 },	
  				[1518] = { ["rate"] = 100 },	
    			}
			  
-- Time in milliseconds to transmit for
local duration			= 10000;
local confirmDuration	= 60000;			-- If set to 0, Confirm run is skipped.
local pauseTime			= 3000;
local graceTime			= 5000;				-- Ramp Up grace time
local numIterations		= 20;	 			-- Maximum number of divide n conquer iterations
local retryCount		= 3;				-- Number of Trial retries if Tx underrun happens
local initialRate		= 100;
--
-- Test tolerance settings
local loss_tol          = 0.10;
local seekPrec			= 0.10;				-- Divide n conquer seek precision 

-- define the ports in use TODO: Use all ports configured in PktGen instead.
local sendport			= "0";
local recvport			= "1";

-- Port config to use TODO: Use json config file instead. 
-- Flows ~ num_ip * num_udpp * 4. Max 10M flows. Requires longer RampUp time in some cases.
local num_ip		= 10; 					-- Number of IPs per interface, max 253. Set DUT IP at .254.
local num_udpp		= 100;					-- Number of UDP ports used per IP, max 9999.  
local srcip			= "192.168.10.1";
local max_srcip		= "192.168.10."..num_ip;
local dstip			= "192.168.20.1";
local max_dstip		= "192.168.20."..num_ip;
local netmask		= "/24";
local p0_udpp_src	= 10000;
local p0_udpp_dst	= 20000;
local p1_udpp_src	= 30000;
local p1_udpp_dst	= 40000;
--local p0_destmac	= "b4:96:91:00:00:01";	-- Comment out to test Pktgen loopback
--local p1_destmac	= "b4:96:91:00:00:02";	-- Set to correct DUT MACs

-- Global variables
local sent			= 0;
local loss          = 0;
local loss_limit	= 0;
local adjust_rate	= 0;

local function init()
	if SHOW_STATUS 
	then 
		pktgen.screen("on");
	end
	pktgen.page("0");
	pktgen.pause("[  INFO ] Starting RFC-2544 script\n",5000);
	print("[NOTICE ] See test log for results: "..logfile);
end

printf = function(s,...)
           print(s:format(...));
         end 

sprintf = function(s,...)
           return s:format(...)
         end 

dprint = function(...)
	   if DEBUG 
	   then
		   print(...);
	   end
         end 

function Round(num, dp)
    local mult = 10^(dp or 0)
    return math.floor(num * mult + 0.5)/mult
end

local function setupTraffic()
	pktgen.reset("all");

	pktgen.set("all", "burst", 64);

	pktgen.set_type("all", "ipv4");
	pktgen.set_proto(sendport..","..recvport, "udp");

	-- Set Single packet profile just for visibility in main screen
	pktgen.set_ipaddr(sendport, "dst", dstip);
	pktgen.set_ipaddr(sendport, "src", srcip..netmask);

	pktgen.set_ipaddr(recvport, "dst", srcip);
	pktgen.set_ipaddr(recvport, "src", dstip..netmask);
	
	pktgen.set(sendport, "sport", p0_udpp_src); 
	pktgen.set(sendport, "dport", p0_udpp_dst); 
	pktgen.set(recvport, "sport", p1_udpp_src); 
	pktgen.set(recvport, "dport", p1_udpp_dst); 


	if p0_destmac then pktgen.set_mac(sendport, "dst", p0_destmac); end
	if p1_destmac then pktgen.set_mac(recvport ,"dst", p1_destmac); end

	-- Set up Range configuration
	pktgen.range.ip_proto("all", "udp");

	if p0_destmac then pktgen.range.dst_mac(sendport, "start", p0_destmac); end
	if p1_destmac then pktgen.range.dst_mac(recvport, "start", p1_destmac); end

	pktgen.range.dst_ip(sendport, "start", dstip);
	pktgen.range.dst_ip(sendport, "inc", "0.0.0.1");
	pktgen.range.dst_ip(sendport, "min", dstip);
	pktgen.range.dst_ip(sendport, "max", max_dstip);

	pktgen.range.src_ip(sendport, "start", srcip);
	pktgen.range.src_ip(sendport, "inc", "0.0.0.1");
	pktgen.range.src_ip(sendport, "min", srcip);
	pktgen.range.src_ip(sendport, "max", max_srcip);

	pktgen.range.dst_ip(recvport, "start", srcip);
	pktgen.range.dst_ip(recvport, "inc", "0.0.0.1");
	pktgen.range.dst_ip(recvport, "min", srcip);
	pktgen.range.dst_ip(recvport, "max", max_srcip);

	pktgen.range.src_ip(recvport, "start",dstip);
	pktgen.range.src_ip(recvport, "inc", "0.0.0.1");
	pktgen.range.src_ip(recvport, "min", dstip);
	pktgen.range.src_ip(recvport, "max", max_dstip);

	pktgen.range.src_port(sendport, "start", p0_udpp_src);
	pktgen.range.src_port(sendport, "inc", 1);
	pktgen.range.src_port(sendport, "min", p0_udpp_src);
	pktgen.range.src_port(sendport, "max", p0_udpp_src + num_udpp);

	pktgen.range.dst_port(sendport, "start", p0_udpp_dst);
	pktgen.range.dst_port(sendport, "inc", 1);
	pktgen.range.dst_port(sendport, "min", p0_udpp_dst);
	pktgen.range.dst_port(sendport, "max", p0_udpp_dst + num_udpp);

	pktgen.range.src_port(recvport, "start", p1_udpp_src);
	pktgen.range.src_port(recvport, "inc", 1);
	pktgen.range.src_port(recvport, "min", p1_udpp_src);
	pktgen.range.src_port(recvport, "max", p1_udpp_src + num_udpp);

	pktgen.range.dst_port(recvport, "start", p1_udpp_dst);
	pktgen.range.dst_port(recvport, "inc", 1);
	pktgen.range.dst_port(recvport, "min", p1_udpp_dst);
	pktgen.range.dst_port(recvport, "max", p1_udpp_dst + num_udpp);

	pktgen.range.ttl("all", "start", 64);
	pktgen.range.ttl("all", "inc", 0);
	pktgen.range.ttl("all", "min", 64);
	pktgen.range.ttl("all", "max", 64);

	pktgen.set_range("all", "on");

	pktgen.pause("[  INFO ] Starting Test\n",2000);

end

local function logResult(s)
	print(s);
        file:write(s.."\n");
        file:flush();
end

local function getLinkSpeed()
	local port_link;

        port_link = string.match(pktgen.linkState(sendport)[tonumber(sendport)],"%d+");

	return tonumber(port_link);
end

-- Get Max PPS for one interface
local function getMaxPPS(pkt_size)
	local max_pps, port_link;

    port_link = string.match(pktgen.linkState(sendport)[tonumber(sendport)],"%d+");
	max_pps = Round((port_link * 10^6)/((pkt_size + 20) * 8)); 

	return max_pps;
end

local function getDuration(pkt_size, set_rate, num_pkts)
	local test_dur, max_pps;

	max_pps = getMaxPPS(pkt_size) * 2; --Bidir
    test_dur = num_pkts / (max_pps * (set_rate/100)); 

	return(test_dur * 1000);
end

local function checkRate(pkt_size, set_rate, duration, num_pkts)
	local calc_pps, max_pps, test_rate, calc_duration, rate_ok = false;
	
	-- Verify that actual duration is not much lower than set duration
	-- This is when PktGen is unable to produce Tx Rate.
	-- Otherwize use the actual duration to calculate test rate
	calc_duration = getDuration(pkt_size, set_rate, num_pkts);
	if calc_duration > (duration - 0.1) 
	then
		duration = calc_duration;
	end

	max_pps = (getMaxPPS(pkt_size) * 2); --x2 for bidir
    calc_pps = (num_pkts / ((duration) / 1000)); 
    test_rate = Round(((calc_pps / max_pps)*100),3);
    set_rate = Round(set_rate, 3);
	dprint("[ DEBUG ] Link:"..(getLinkSpeed()//1000).."GBE | Max MPPS:"..Round(max_pps/10^6,2).." | Test MPPS:"..Round(calc_pps/10^6,2).." | Dur: "..duration.." | Calc Rate:"..test_rate.." < "..set_rate..":Set Rate");

	if (test_rate - set_rate) < -0.1
	then
		logResult("[WARNING] Set rate "..set_rate.." is not reached ("..test_rate.." < "..set_rate.."), PktGen is unable to reach Tx target");
		adjust_rate = test_rate;
		rate_ok = false;
	else
		adjust_rate = set_rate;
		rate_ok = true;
	end

	return(rate_ok);
end

local function getTxPkts(set_rate, pkt_size, test_dur)
	local max_pps, num_pkts;

	max_pps = getMaxPPS(pkt_size); 
	num_pkts = Round((max_pps  * (set_rate/100))*(test_dur/1000));
	dprint("[ DEBUG ] getTxPkts Rate: "..set_rate.."% | Num Pkts:"..num_pkts.." | Duration:"..test_dur/1000);

	return num_pkts;
end

local function getPPS(num_pkts, test_dur)
	local test_pps;
        
	test_pps = Round(num_pkts / (test_dur/1000));

	return test_pps;
end

local function getMbpsL2(num_pkts, pkt_size, test_dur)
	local test_mbps, test_pps;
        
	test_pps = getPPS(num_pkts, test_dur);
	test_mbps = Round((test_pps * (pkt_size * 8))/10^6,3);

	return test_mbps;
end

local function getMbpsL1(num_pkts, pkt_size, test_dur)
	local test_mbps, test_pps;
        
	test_pps = getPPS(num_pkts, test_dur);
	test_mbps = Round((test_pps * ((pkt_size + 20)* 8))/10^6,3);

	return test_mbps;
end


local function printLine()
	print("-------------------------------------------------------------------------------------------------------------");
	file:write("--------------------------------------------------------------------------------------------------------------\n");
	file:flush();
end


local function runTrial(pkt_size, rate, duration, count, retry)
	local num_tx, num_tx1, num_rx, num_dropped, tx_mpps, rx_mpps, tx_gbps1, rx_gbps1, tx_gbps2, rx_gbps2, rate_notif = " ";

	pktgen.clr();
	if count == 1 and retry == 1 
	then
		pktgen.page("range");
	end
	pktgen.set("all", "rate", rate);
	pktgen.set("all", "size", pkt_size);

	pktgen.range.pkt_size("all", "start", pkt_size);
	pktgen.range.pkt_size("all", "inc", 0);
	pktgen.range.pkt_size("all", "min", pkt_size);
	pktgen.range.pkt_size("all", "max", pkt_size);
	if count == 1 and retry == 1 
	then
		pktgen.pause("[  INFO ] IP Range , setting "..pkt_size.."B\n",5000);
		pktgen.page("0");
	end

	printLine();
	print("[  INFO ] Starting Test...");

	-- Use RAMPUP with graceTime seconds of traffic before test or just send exact number of packets
	if USE_RAMPUP 
	then
		pktgen.set("all", "count", 0);
		pktgen.start("all"); 			-- Bidir
		print("[  INFO ] Ramping up Flows...");
		pktgen.delay(graceTime);
		pktgen.clr();
	else
		pktgen.set("all", "count", getTxPkts(rate, pkt_size, (duration)));
		pktgen.start("all"); 			-- Bidir
		pktgen.delay(250); 				-- Extra time to account for start stop events
	end
	pktgen.delay(duration);
	pktgen.stop("all");

	pktgen.delay(pauseTime);
	pktgen.pause("[  INFO ] Waiting for idling packets...\n",2000);

	statTx = pktgen.portStats(sendport, "port")[tonumber(sendport)];
	statRx = pktgen.portStats(recvport, "port")[tonumber(recvport)];
	
	-- Check that port Tx opackets are within reason.
	-- This happens if one interface LAGs behind, will result in Tx underruns in iterations.
	-- Included headroom for RAMP-UP mode as it is always lagging for one interface when test stops
	num_tx = statTx.opackets;
	num_tx1 = statRx.opackets;
	if (num_tx + 5000) < num_tx1 
	then
		print("[WARNING] Port:"..sendport.." Tx underruns.Results might be inconclusive.("..Round(num_tx - num_tx1)..")");
		dprint("[ DEBUG ] CPU probably overloaded.");

	elseif (num_tx1 + 5000) < num_tx 
	then
		print("[WARNING] Port:"..recvport.." Tx underruns. Results might be inconclusive.("..Round(num_tx1 - num_tx)..")");
		dprint("[ DEBUG ] CPU probably overloaded.");
	else
		dprint("[ DEBUG ] No Tx underruns are detected.");
	end

	num_tx = num_tx + num_tx1;
	num_rx = statRx.ipackets + statTx.ipackets;
	num_dropped = num_tx - num_rx;
	sent = num_tx;
	loss = Round((1-(num_rx/num_tx))*100,3);
	loss_limit = Round(sent*(loss_tol/100));
	
	
	-- checkRate: Validate PktGen Tx rate and duration to get adjusted rate if tx underruns happens
	if checkRate(pkt_size, rate, duration, num_tx)
	then
		rate_notif = " "; 
	else
		print("[NOTICE ] Actual Rate is: "..adjust_rate.."%");
		rate = adjust_rate;
		rate_notif = "!"; -- Mark PktGen Max rate in table
	end
	duration = getDuration(pkt_size, rate, num_tx);

	tx_mpps = getPPS(num_tx, duration)/10^6;
	rx_mpps = getPPS(num_rx, duration)/10^6;
	tx_gbps1 = getMbpsL1(num_tx, pkt_size, duration)/1000;
	rx_gbps1 = getMbpsL1(num_rx, pkt_size, duration)/1000;
	tx_gbps2 = getMbpsL2(num_tx, pkt_size, duration)/1000;
	rx_gbps2 = getMbpsL2(num_rx, pkt_size, duration)/1000;

	logResult(sprintf("[ %4dB ] Trial %5s.%1d   | Rate:%s %-6.2f%%  | Pkt Size: %4dB | Dur.: %3.2fs | Loss Tol.: %-5.3f%%", pkt_size, tostring(count), retry, rate_notif, rate, pkt_size, Round(duration/1000,3), loss_tol));
	logResult(sprintf("[ %4dB ] Tx: %-11d | Rx: %-11d | Drop: %-13d (%-6.3f%%)  | Loss Lim.: %-10.0f", pkt_size, num_tx, num_rx, num_dropped, loss, loss_limit));
	logResult(sprintf("[ %4dB ] MPPS Tx: %-06.2f | MPPS Rx: %-06.2f | Gbps Tx L1/L2: %-6.2f / %-6.2f | Gbps Rx L1/L2: %-6.2f / %-6.2f", pkt_size, tx_mpps, rx_mpps, tx_gbps1, tx_gbps2, rx_gbps1, rx_gbps2));

	pktgen.delay(pauseTime);

	return num_dropped;
end

local function runThroughputTest(pkt_size)
	local num_dropped, max_rate, min_rate, trial_rate, max_min_diff, no_adjust = true;
	max_rate = pktgen_limit[pkt_size].rate;
	min_rate = 1;
	trial_rate = initialRate;
	if trial_rate > max_rate then trial_rate = max_rate; end 

	for count=1, numIterations, 1
	do
		for retry=1, retryCount, 1
		do
			num_dropped = runTrial(pkt_size, trial_rate, duration, count, retry);
			if (adjust_rate + 0.1) >= trial_rate 
			then 
				break; 
			else
				-- If adjusted Tx and at 100% whilst packet drops, 
				-- DUT is the bottleneck so no retries is needed.
				if trial_rate == 100 and num_dropped > loss_limit 
				then
					break;	
				end
			end

			if retry < retryCount 
			then 
				dprint("[ DEBUG ] Retrying Trial...");
				pktgen.delay(pauseTime);
			end
		end
		if (adjust_rate + 0.1) < trial_rate 
		then
			trial_rate = adjust_rate; 
			max_rate = trial_rate;
			no_adjust = false;
		else	
			no_adjust = true;
		end

		if num_dropped <= loss_limit 
		then
			-- Undershoot
			if no_adjust 
			then 
				min_rate = trial_rate;
			end
			--if min_rate == pktgen_limit[pkt_size].rate then break; end 
			if min_rate == max_rate then break; end 
			trial_rate = min_rate + 1 + ((max_rate - min_rate)/2);	-- Add +1 to get to 100% if initial rate is set to a lower value.
			if trial_rate > max_rate then trial_rate = max_rate; end 
		else
			-- Overshoot
			max_rate = trial_rate;
			if count > 2 
			then
				trial_rate = max_rate - ((max_rate - min_rate)/2);
			else
				-- First 2 iterations trying to do rapid find by using loss% to set trial rate 
				trial_rate = trial_rate - ((trial_rate/100)*loss);
			end
		end

		max_min_diff = max_rate - min_rate;
		dprint("[ DEBUG ] Max - Min rate diff: "..Round(max_min_diff,5));
		if max_min_diff <= seekPrec
		then
			dprint("[ DEBUG ] Stopping seek iterations, reached seek Precision limit");
			break;
		end
	end

	if confirmDuration > 0
	then
		-- Ensure we test confirmation run with the last succesfull drop rate
		trial_rate = min_rate;

		-- confirm throughput rate for at least 60 seconds
		num_dropped = runTrial(pkt_size, trial_rate, confirmDuration, "Final", 0);
		
		printLine();
		if num_dropped <= loss_limit
		then
			print("[RESULT ] Max rate for packet size "  .. pkt_size .. "B is: " .. adjust_rate.."%");
			file:write("[RESULT ] Max rate for packet size "  .. pkt_size .. "B is: " .. adjust_rate .. "%\n\n");
		else
			print("[WARNING] Max rate of " .. trial_rate .. "% could not be confirmed for 60 seconds as required by rfc2544.");
			file:write("[WARNING] Max rate of " .. trial_rate .. "% could not be confirmed for 60 seconds as required by rfc2544." .. "\n\n");
		end
		file:flush();
	end
	pktgen.delay(pauseTime);
end

function main()
	local dummy;
	file = io.open(logfile, "w");
	setupTraffic();
	pktgen.pause("[  INFO ] Priming DUT...\n",1000);
	dummy = runTrial(512, 1, 5000, "Prime",0);
	file:write("\n");
        if dummy >= (sent * loss_tol)
	then
	 	pktgen.pause("[WARNING] DUT seem to be dropping packets, stopping test.\n",1000);
	else
		pktgen.pause("[  INFO ] Starting Trial Runs ...\n",3000);
		for _,size in pairs(pkt_sizes)
		do
			runThroughputTest(size);
	  	end
	end
	printLine();
	file:close();
	print("[NOTICE ] See test log for results: "..logfile);
end
init();
main();
