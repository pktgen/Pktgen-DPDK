.. _lua:

Using Lua with Pktgen
=====================

Lua is a high level dynamic programming language. It is small and lightweight
and can easily be embedded in applications written in other languages. It is
also suitable for loading and wrapping dynamic libraries.

Lua is used in ``pktgen`` to script and configure the application and also to
plug into DPDK functions to expose configuration and statistics.


The following are some of the examples included in the ``test`` directory of
``pktgen`` repository.



Example: Hello World
--------------------

A simple "hello world" example to ensure that everything is working correctly:


.. literalinclude:: ../../test/hello-world.lua
   :language: lua



Example: Info
-------------

A simple example to print out some metadata and configuration information from
``pktgen``:

.. literalinclude:: ../../test/info.lua
   :language: lua


Example: More Info
------------------

Another example to print out data from a running ``pktgen`` instance:

.. literalinclude:: ../../test/test3.lua
   :language: lua


Example: Sequence
-----------------

An example to set a packet sequence:

.. literalinclude:: ../../test/set_seq.lua
   :language: lua
   :tab-width: 4


Example: Main
-------------

A more complex example showing most of the features available via the Lua interface and also show interaction with the user:

.. literalinclude:: ../../test/main.lua
   :language: lua
