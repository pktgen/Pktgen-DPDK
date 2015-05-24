.. _scripts:


Running Script Files
====================

Pktgen can read and run files with default values and configurations via
the ``-f`` commandline option (:ref:`usage_pktgen`).

These files can either be ``.pkt`` files with Pktgen :ref:`runtime commands
<commands>` as shown in the previous section or ``.lua`` files with the same
commands and options in Lua syntax.

For example here is a ``pktgen`` instance that read a ``.pkt`` file::

   ./app/pktgen -c 0x1f -n 3 --proc-type auto --socket-mem 128,128 -- \
                -P -m "[1:3].0, [2:4].1" -f test/set_seq.pkt

Where the ``test/set_seq.pkt`` (included in the ``pktgen`` repository) is as
follows::

   seq 0 all 0000:4455:6677 0000:1234:5678 10.11.0.1 10.10.0.1/16 5 6 ipv4 udp 1 128
   set all seqCnt 1

The Lua version (``test/set_seq.lua`` in ``pktgen`` repository) is clearer and
allows extension through standard Lua or user defined functions:

.. literalinclude:: ../../test/set_seq.lua
   :language: lua
   :lines: 3-
   :tab-width: 4

The Lua interface is explained in the next section :ref:`lua`.
