#!/bin/bash
#
#sudo ./builddir/examples/pktperf/pktperf -l 1,2-5,6-9 -a 03:00.0 -a 05:00.0 -- -r $1 -m "2-3:4-5.0" -m "6-7:8-9.1"
sudo ./builddir/examples/pktperf/pktperf -l 1,2-7,8-13 -a 03:00.0 -a 05:00.0 -- -r $1 -m "2-4:5-7.0" -m "8-10:11-13.1"
#sudo gdb -arg ./builddir/examples/pktperf/pktperf -l 2,4-6,14-16 -a 03:00.0 -a 82:00.0 -- -r $1 -m "4-6.0" -m "14-16.1"
