.. _socket:

Socket Support for Pktgen
=========================

Pktgen provides a TCP socket connection to allow you to control it from a
remote console or program.

The TCP connection uses port 22022, 0x5606, and presents a Lua command shell
interface.

If you telnet on port 22022 to a machine running ``pktgen`` you will get a Lua
command shell like interface. This interface does not have a command line
prompt, but you can issue Lua code or load script files from the local disk of
the machine. You can also send programs to the remote ``pktgen`` machine to
load scripts and run scripts.

Another way to connect remotely to ``pktgen`` is to use the ``socat`` program
on a Linux machine::

   $ socat -d -d READLINE TCP4:localhost:22022

This will create a connection and then wait for Lua command scripts. You can
also send ``pktgen`` a command script file and display the output::


   $ socat - TCP4:localhost:22022 < test/hello-world.lua

   Lua Version      : Lua 5.3
   Pktgen Version   : 2.9.0
   Pktgen Copyright : Copyright (c) `<2010-2015>`, Wind River Systems, Inc.
   Pktgen Authors   : Keith Wiles @ Wind River Systems

   Hello World!!!!

Where the the ``test/hello-world.lua`` looks like this:

.. literalinclude:: ../../test/hello-world.lua
   :language: lua

Here is another ``socat`` example which loads a file from the local disk where
``pktgen`` is running and then we execute the file with a user defined
function::

   $ socat READLINE TCP4:172.25.40.163:22022
   f,e = loadfile("test/hello-world.lua")
   f()
   Lua Version      : Lua 5.3
   Pktgen Version   : 2.9.0
   Pktgen Copyright : Copyright (c) `<2010-2015>`, Wind River Systems, Inc.
   Pktgen Authors   : Keith Wiles @ Wind River Systems

   Hello World!!!!
   <Control-D>


You can also just send it commands via echo::

   $ echo "f,e = loadfile('test/hello-world.lua'); f();" \
          | socat - TCP4:172.25.40.163:22022
   Lua Version      : Lua 5.3
   Pktgen Version   : 2.9.0
   Pktgen Copyright : Copyright (c) `<2010-2015>`, Wind River Systems, Inc.
   Pktgen Authors   : Keith Wiles @ Wind River Systems

   Hello World!!!!
