Pktgen - Traffic Generator powered by DPDK
=====================================================

**Pktgen is a traffic generator powered by DPDK at wire rate traffic with 64 byte frames.**

** (Pktgen) Sounds like 'Packet-Gen'**

---
```
**Copyright &copy; \<2010-2023\>, Intel Corporation. All rights reserved.**

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

- Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

SPDX-License-Identifier: BSD-3-Clause

Pktgen: Created 2010-2023 by Keith Wiles @ Intel.com
```
---
Pktgen documentation can be found here: https://pktgen.github.io/Pktgen-DPDK/

---
Using the tools/run.py script to setup and run pktgen with different configurations. The configuration files are located in the cfg directory with filenames ending in .cfg.

To use a configuration file;
```
$ ./tools/run.py -s default  # to setup the ports and attach them to DPDK (only needed once per boot)

$ ./tools/run.py default     # Run the default configuration
```
The configuration files are python scripts or a set of variables that run.py uses to initialize and run pktgen.
Here is an example of the default.cfg file:

```
	description = 'A Pktgen default simple configuration'

	# Setup configuration
	setup = {
	    'exec': (
		'sudo', '-E'
		),

	    'devices': (
		    '81:00.0', '81:00.1', '81:00.2', '81:00.3',
		    '83:00.0', '83:00.1', '83:00.2', '83:00.3'
		    ),
	    # UIO module type, igb_uio, vfio-pci or uio_pci_generic
	    'uio': 'vfio-pci'
	    }

	# Run command and options
	run = {
	    'exec': (
		'sudo', '-E'
		),

	    # Application name and use app_path to help locate the app
	    'app_name': 'pktgen',

	    # using (sdk) or (target) for specific variables
	    # add (app_name) of the application
	    # Each path is tested for the application
	    'app_path': (
		'./app/%(target)s/%(app_name)s',
		'%(sdk)s/%(target)s/app/%(app_name)s',
		),

	    'cores': '14,15-22',
	    'nrank': '4',
	    'proc': 'auto',
	    'log': '7',
	    'prefix': 'pg',

	    'blocklist': (
		#'81:00.0', '81:00.1', '81:00.2', '81:00.3',
		#'83:00.0', '83:00.1', '83:00.2', '83:00.3',
		'81:00.2', '81:00.3',
		'83:00.2', '83:00.3'
		),

	    'opts': (
		'-T',
		'-P',
		),
	    'map': (
		'[15:16].0',
		'[17:18].1',
		'[19:20].2',
		'[21:22].3'
		),

	    'theme': 'themes/black-yellow.theme'
	}
```

------------------------------------------------------------------------------------------
-- Two Pktgens running on the same machine with connected via a loopback ports

Look at the two new files pktgen-1.cfg and pktgen-2.cfg in the cfg directory for some help on
the configuration to run two pktgens at the same time on the same machine.

------------------------------------------------------------------------------------------
-- Socket Support for Pktgen.

Pktgen has a TCP socket connection to allow you to control Pktgen from a remote
program or console. The TCP connection is using port 0x5606 or 22022 and presents
a Lua command shell interface. If you telnet to the machine running Pktgen on port
22022 you will get a lua command shell like interface. This interface does not have
a command line prompt, but you can issue Lua code or load script files from the local
disk of the machine. You can also send programs to the remote Pktgen machine to
load scripts from a remote location.

One method to connect to Pktgen is using telnet, but another method would be to
use 'socat' program on a Linux machine. The socat program is very powerfull application
and can do a lot of things. I used socat to debug Pktgen using the following
command, which gives me a readline inteface to Pktgen's socket interface.
Remember to use the '-G' option or -g host:port pktgen option, then make sure you
use the same address in the socat line.

```
$ socat -d -d READLINE TCP4:localhost:22022
```

'You will see socat create the connection and then wait for Lua command scripts for you'
To exit this command type Control-D to exit and close the connection.

You can also just send Pktgen a script file and display the ouptut.

---------
    $ socat - TCP4:localhost:22022 < test/hello-world.lua

    Lua Version: Lua 5.3
    Pktgen Version : 3.6.1
    Pktgen Copyright : Copyright(c) `<2010-2023>`, Intel Corporation
    Pktgen Authors : Keith Wiles @ Intel Corporation

Hello World!!!!
--------

Here is the program I sent Pktgen:

    $ cat test/hello-world.lua
    package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

    printf("Lua Vesrion: %s\n", pktgen.info.Lua_Version);
    printf("Pktgen Version : %s\n", pktgen.info.Pktgen_Version);
    printf("Pktgen Copyright : %s\n", pktgen.info.Pktgen_Copyright);
    printf("Pktgen Authors : %s\n", pktgen.info.Pktgen_Authors);

    printf("\nHello World!!!!\n");
-----------

Here is a command from my Mac Book pro laptop, which loads a file from the local
disk where Pktgen is running and then we execute the file with 'f()'.

------------------

    $ socat READLINE TCP4:172.25.40.163:22022
    f,e = loadfile("test/hello-world.lua")
    f()
    Lua Version: Lua 5.3
    Pktgen Version : 3.6.1
    Pktgen Copyright : Copyright(c) `<2010-2023>`, Intel Corporation
    Pktgen Authors : Keith Wiles @ Intel Corporation

    Hello World!!!!
    <Control-D>
------------------

You can also just send it commands via echo.

-----------------

    $ echo "f,e = loadfile('test/hello-world.lua'); f();"| socat - TCP4:172.25.40.163:22022
    Lua Version: Lua 5.3
    Pktgen Version : 3.6.1
    Pktgen Copyright : Copyright(c) `<2010-2023>`, Intel Corporation
    Pktgen Authors : Keith Wiles @ Intel Corporation

    Hello World!!!!
----------------------

Keith Wiles @ Intel Corporation
