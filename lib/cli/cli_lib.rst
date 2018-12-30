..  BSD LICENSE
   Copyright(c) <2016-2019> Intel Corporation. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
   * Neither the name of Intel Corporation nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

CLI library guide
=================

CLI stands for "Command Line Interface".

This chapter describes the CLI library which is a part of the Data Plane
Development Kit (DPDK). The CLI is a workalike replacement for cmdline library
in DPDK and has a simpler interface and programming model plus it is dynamic.

The primary goal of CLI is to allow the developer to create commands quickly
and with very little compile or runtime configuration. Using standard Unix*
like constructs which are very familar to the developer. Allowing the developer
to construct a set of commands for development or deployment of the application.

The CLI design uses a directory like design instead of a single level command
line interface. Allowing the developer to use a directory style solution to
controlling a DPDK application. The directory style design is nothing new, but
it does have some advantages over a single level command structure.

One advantage allows the directory path for the command to be part of the
information used in executing the command. The next advantage is creating
directories to make a hierarchy of commands, plus allowing whole directroy
trees to dynamicly come and go as required by the developer.

Some of the advantages are:

 * CLI has no global variable other then the single thread variable called *this_cli* which can only be accessed from the thread which created the CLI instance.
 * CLI supports commands, files, aliases, directories.
    - The alias command is just a string using a simple substitution support for other commands similar to the bash shell like alias commands.
    - Files can be static or dynamic information, can be changed on the fly and saved for later. The file is backed with a simple function callback to allow the developer to update the content or not.
 * Added support for color and cursor movement APIs similar to Pktgen if needed by the developer.
 * It is a work alike replacement for cmdline library. Both cmdline and CLI can be used in the same application if care is taken.
 * Uses a simple fake like directory layout for command and files. Allowing for command hierarchy as path to the command can allow for specific targets to be identified without having to state it on the command line.
 * Has auto-complete for commands, similar to Unix/Linux autocomplete and provides support for command option help as well.
 * Callback functions for commands are simply just argc/argv like functions.
    - The CLI does not convert arguments for the user, it is up to the developer to decode the argv[] values.
    - Most of the arguments converted in the current cmdline are difficult to use or not required as the developer just picks string type and does the conversion himself.
 * Dynamically be able to add and remove commands, directories, files and aliases, does not need to be statically compiled into the application.
 * No weird structures in the code and reduces the line count for testpmd from 12K to 4.5K lines. I convert testpmd to have both CMDLINE and CLI with a command line option.
 * Two methods to parse command lines, first is the standard argc/argv method in the function.
    - The second method is to use a map of strings with simple printf like formatting to detect which command line the user typed.
    - An ID value it returned to the used to indicate which mapping string was found to make the command line to be used in a switch statement.
 * Environment variable support using the **env** command or using an API.
 * Central help support if needed (optional).

Overview
--------

The CLI library is a simple set of APIs which allow the developer to quickly
create a set of commands using a simple programming interface already familar
to the developer.

One of the big advantages of CLI over Cmdline is it is dynamic, which means
nodes or items can be added and removed on the fly. Which allows adding
new directories, file or commands as needed or removing these items at runtime.
The CLI has no global modifiable variables except for the one global pointer
which is a thread based variable. Allowing the developer to have multiple CLI
instances running at the same time on different threads if needed.

Another big advantage is the calling of the backend function to support a
command is very familar to developers as it is basically just a argc/argv
style command and the developer gets the complete command line. The function
as access to the global thread variable called **this_cli** pointing to the
struct cli variable.

.. code--block:: c

    /* Show command returns 0 on OK and -1 on error */
	int show_cmd(int argc, char **argv);

Mapping commands
----------------

One other big advantage is the use of MAP structures, to help identify commands
quickly plus allowing the developer to define new versions of commands and
be able to identify these new versions using a simple identifier value.

The format of the struct cli_map is:

.. code-block:: c

    struct cli_map show_map[] = {
        /* Index value, Mapping string */
        { 10, "show" },
        { 20, "show %s" },
        { 30, "show %P stats" },
        { 40, "show %P %|link|errors|missed stats" },
        { 0, NULL}
    }

The map is just an array of struct cli_map entries with a unique index value
and mapping string. The index value can be any value the developer wants. As
the index value is used to identify the given map string.

The map string is a special formatted string similar to sprintf(), but the
format values for % is different. Please look at the cli_mapping() function
docs for more information. The %s is for any string and %P is used to a portlist
format e.g. 1-3,5-7,9 as used for DPDK command line notation.

The above array is parsed to match the command line from the user. The first
map string that matches the user input will be returned from the call to
cli_mapping() function.

Constant values are required in the command as in index 30 'stats'. The index
40 is using a variable fixed set of strings option, which means one of these
fixed strings must match in that position.

Another advantage of CLI is how simple it is to add new directroies, files and
commands for user development. To add a command a developer needs to add an
entry to the cli_tree structure and create a function using the above
prototype format.

.. code-block:: c

    struct cli_tree my_tree[] = {
        c_dir("/bin"),
        c_cmd("hello", hello_cmd, "simple hello world command"),
        c_cmd("show",  show_cmd,  "Show system information"),
        c_end()
    };

The cli_tree structure is made with unions and the c_dir(), c_cmd() and c_end()
help initialize the structure easily for the developer. The help and show
commands above use the simple argc/argv prototype above.

Only two things are required to create a command a cli_tree entry and a function
to call. Using the cli_map and other structures are optional to make adding
simple commands quick and easy. The call the cli_create() command or one of its
helper functions cli_create_XYZ(). If have a function per command then using the
mapping structure is optional, unless you want to have CLI parse and map
commands to the exact entries. If cli_map is not used then the developer needs
to decode the argc/argv to determine the command requests.

The argc/argv is exactly like the standard usage in a Unix* system, which allows
for using getopt() and other standard functions. The Cmdline structures and
text conversions were defined at compile time in most cases, but in CLI the
command routine is passed the argc/argv information to convert the strings as
needed. The cli variable being a thread Local Storage (TLS) all user routines
can access **this_cli** to gain access to the CLI structure if required at all.

Environment variables
---------------------

The user can also set environment variables using the **env** command. These
variables are also parsed in the command line as direct substitutions.

Another special file is a string file, which can be used as an environment
variable. When the variable is asked for the variable asks a function to return
the string. The value of the string normally a system value or a generated
value. These types of environment variables can not be set from the command
line as a function pointer needs to be given. The c_str() macro helps in
setting up these environment variables via the cli_tree structure.

The special file backed environment variable can be deleted, but can not be
restored without a reboot or some other command puting that variable back into
the environment.

Environment variables are denoted by a $(foo) like syntax and are expanded at
the time of execution each time the command line is executed. Which means
history lines with environment variables will be expanded again.

Simple Files
------------

The CLI system also has support for simple files along with alias like commands.
These simple files are backed by a function call and the other commands can read
these files to get constant data or generated data depending on how the backend
function works.

Alias commands
--------------
The alias commands are fixed strings which are executed instead of a function
provided by the developer. If the user has more arguments these are appended
to the alias string and processed as if typed on the command line. Also the
environment variables are expanded at execution time.

.. note::

   The CLI library was designed to be used in production code and the Cmdline
   was not validated to the same standard as other DPDK libraries. The goal
   is to provide a production CLI design.

The CLI library supports some of the features of the Cmdline library such as,
completion, cut/paste and some other special bindings that make configuration
and debug faster and easier.

The CLI desin uses some very simple VT100 control strings for displaying data
and accepting input from the user. Some of the control strings are used to
clear the screen or line and position the cursor on a VT100 compatible terminal.
The CLI screen code also supports basic color and many other VT100 commands.

The example application also shows how the CLI application can be extended to
handle a list of commands and user input.

The example presents a simple command prompt **DPDK-cli:/>** similar to a Unix*
shell command along with a directory like file system.

Some of the **default** commands contained under /sbin directory are:

 * **ls**: list the current or provided directory files/commands.
 * **cd**: Change directory command.
 * **pwd**: print out the current working directory.
 * **history**: List the current command line history if enabled.
 * **more**: A simple command to page contents of files.
 * **help**: display a the help screen.
 * **quit**: exit the CLI application, also **Ctrl-x** will exit as well.
 * **mkdir**: add a directory to the current directory.
 * **delay**: wait for a given number of microseconds.
 * **sleep**: wait for a given number of seconds.
 * **rm**: remove a directory, file or command. Removing a file will delete the data.
 * **cls**: clear the screen and redisplay the prompt.
 * **version**: Display the current DPDK version being used.
 * **path**: display the current search path for executable commands.
 * **cmap**: Display the current system core and socket information.
 * **hugepages**: Display the current hugepage information.
 * **sizes**: a collection system structure and buffer sizes for debugging.
 * **copyright**: a file containing DPDK copyright information.
 * **env**: a command show/set/modify the environment variables.

 * **ll**: an alias command to display long ls listing **ls -l**
 * **h**: alias command for **history**
 * **hello**: a simple Hello World! command.
 * **show**: has a number of commands using the map feature.

Under the /data directory is:

 * **pci**: a simple example file for displaying the **lspci** command in CLI.

.. note::

   To terminate the application, use **Ctrl-x** or the command **quit**.

Auto completion
---------------

CLI does support auto completion at the file or directory level, meaning the
arguments to commands are not expanded as was done in Cmdline code. The CLI
auto completion works similar to the standard Unix* system by expanding
commands and directory paths. In normal Unix* like commands the user needs to
execute the command asking for help information.

Special command features
------------------------

Using the '!' followed by a number from the history list of commands you can
execute that command again. Or using the UP/Down arrows the user can quickly
find and execute or modify a previous command in history.

The user can also execute host level commands if enabled using the '@' prefix
to a command line e.g. @ls or @lspci or ... line is passed to popen or system
function to be executed and the output displayed on the console if any output.

Compiling the Application
-------------------------

#.  Go to example directory:

.. code-block:: c

   export RTE_SDK=/path/to/rte_sdk
   cd ${RTE_SDK}/examples/cli

#.  Set the target (a default target is used if not specified). For example:

.. code-block:: console

   export RTE_TARGET=x86_64-native-linuxapp-gcc

Refer to the *DPDK Getting Started Guide* for possible RTE_TARGET values.

#.  Build the application:

.. code-block:: console

   make

Running the Application
-----------------------

To run the application in linuxapp environment, issue the following command:

.. code-block:: console

   $ ./build/cli

.. note::
   The example cli application does not require to be run as superuser
   as it does not startup DPDK by calling rte_eal_init() routine. Which means
   it also does not use DPDK features except for a few routines not requiring
   EAL initialization.

Refer to the *DPDK Getting Started Guide* for general information on running applications
and the Environment Abstraction Layer (EAL) options.

Explanation
-----------

The following sections provide some explanation of the code.

EAL Initialization and cmdline Start
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first task is the initialization of the Environment Abstraction Layer (EAL),
if required for the application.

.. code-block:: c

   int
   main(int argc, char **argv)
   {
       if (cli_create_with_tree(init_tree) ==0) {
           cli_start(NULL, 0); /* NULL is some init message done only once */
                               /* 0 means do not use color themes */
           cli_destroy();
       }

The cli_start() function returns when the user types **Ctrl-x** or uses the
quit command in this case, the application exits. The cli_create() call takes
four arguments and each has a default value if not provided. The API used here
is the cli_create_with_tree(), which uses defaults for three of the arguments.

.. code-block:: c

   /**
   * Create the CLI engine
   *
   * @param prompt_func
   *   Function pointer to call for displaying the prompt.
   * @param tree_func
   *   The user supplied function to init the tree or can be NULL. If NULL then
   *   a default tree is initialized with basic commands.
   * @param nb_entries
   *   Total number of commands, files, aliases and directories. If 0 then use
   *   the default number of nodes. If -1 then unlimited number of nodes.
   * @param nb_hist
   *   The number of lines to keep in history. If zero then turn off history.
   *   If the value is CLI_DEFAULT_HISTORY use CLI_DEFAULT_HIST_LINES
   * @return
   *   0 on success or -1
   */
   int cli_create(cli_prompt_t prompt_func, cli_tree_t tree_func,
                       int nb_entries, uint32_t nb_hist);

The cli_create_with_tree() has only one argument which is the structure to use
in order to setup the initial directory structure. Also the wrapper function
int cli_create_with_defaults(void) can be used as well.

Consult the cli.h header file for the default values. Also the alias node is a
special alias file to allow for aliasing a command to another command.

The tree init routine is defined like:

.. code-block:: c

	static struct cli_tree my_tree[] = {
	    c_dir("/data"),
	    c_file("pci", pci_file, "display lspci information"),
	    c_dir("/bin"),
	    c_cmd("hello", hello_cmd, "Hello-World!!"),
	    c_alias("h", "history", "display history commands"),
	    c_alias("ll", "ls -l", "long directory listing alias"),
	    c_end()
	};

	static int
	init_tree(void)
	{
	    /*
	     * Root is created already and using system default cmds and dirs, the
	     * developer is not required to use the system default cmds/dirs.
	     */
	    if (cli_default_tree_init())
	        return -1;

		/* Using NULL here to start at root directory */
	    if (cli_add_tree(NULL, my_tree))
	        return -1;

		cli_help_add("Show", show_map, show_help);

		return cli_add_bin_path("/bin");
	}


The above structure is used to create the tree structure at initialization
time. The struct cli_tree or cli_tree_t typedef can be used to setup a new
directory tree or agument the default tree.

The elements are using a set of macros c_dir, c_file, c_cmd, c_alias and c_end.
These macros help fill out the cli_tree_t structure for the given type of item.

The developer can create his own tree structure with any commands that are
needed and/or call the cli_default_tree_init() routine to get the default
structure of commands. If the developer does not wish to call the default
CLI routine, then he must call the cli_create_root() function first before
adding other nodes. Other nodes can be added and removed at anytime.

CLI Map command support
~~~~~~~~~~~~~~~~~~~~~~~

The CLI command has two types of support to handle arguments normal argc/argv
and the map system. As shown above the developer creates a directory tree and
attaches a function to a command. The function takes the argc/argv as arguments
and the developer can just parse the arguments to decode the command arguments.
Sometimes you have multiple commands or different versions of a command being
handled by a single routine, this is were the map support comes into play.

The map support defines a set of struct cli_map map[]; to help detect the
correct command from the user. In the list of cli_map structures a single
structure contains two items a developer defined index value and a command
strings. The index value is used on the function to identify the specific type
of command found in the list. The string is a special printf like string to
help identify the command typed by the user. One of the first things todo in
the command routine is to call the cli_mapping() function passing in the CLI
pointer and the argc/argv values.The two method can be used at the same time.

The cli_mapping() command matches up the special format string with the values
in the argc/argv array and returns the developer supplied index value or really
the pointer the struct cli_map instance.

Now the developer can use the cli_map.index value in a switch() statement to
locate the command the user typed or if not found a return of -1.

Example:

.. code-block:: c

	static int
	hello_cmd(int argc, char **argv)
	{
	    int i, opt;

	    optind = 1;
	    while((opt = getopt(argc, argv, "?")) != -1) {
	        switch(opt) {
	            case '?': cli_usage(); return 0;
	            default:
	                break;
	        }
	    }

	    cli_printf("Hello command said: Hello World!! ");
	    for(i = 1; i < argc; i++)
	        cli_printf("%s ", argv[i]);
	    cli_printf("\n");

	    return 0;
	}

	static int
	pci_file(struct cli_node *node, char *buff, int len, uint32_t opt)
	{
		if (is_file_open(opt)) {
			FILE *f;

			if (node->file_data && (node->fflags & CLI_FREE_DATA))
				free(node->file_data);

	        node->file_data = malloc(32 * 1024);
			if (!node->file_data)
				return -1;
	        node->file_size = 32 * 1024;
	        node->fflags = CLI_DATA_RDONLY | CLI_FREE_DATA;

			f = popen("lspci", "r");
			if (!f)
				return -1;

			node->file_size = fread(node->file_data, 1, node->file_size, f);

			pclose(f);
	        return 0;
	    }
	    return cli_file_handler(node, buff, len, opt);
	}

	static struct cli_map show_map[] = {
		{ 10, "show %P" },
		{ 20, "show %P mac %m" },
		{ 30, "show %P vlan %d mac %m" },
		{ 40, "show %P %|vlan|mac" },
		{ -1, NULL }
	};

	static const char *show_help[] = {
		"show <portlist>",
		"show <portlist> mac <ether_addr>",
		"show <portlist> vlan <vlanid> mac <ether_addr>",
		"show <portlist> [vlan|mac]",
		CLI_HELP_PAUSE,
		NULL
	};

	static int
	show_cmd(int argc, char **argv)
	{
		struct cli_map *m;
		uint32_t portlist;
		struct ether_addr mac;

		m = cli_mapping(Show_info.map, argc, argv);
		if (!m)
			return -1;

		switch(m->index) {
			case 10:
				rte_parse_portlist(argv[1], &portlist);
				cli_printf("   Show Portlist: %08x\n", portlist);
				break;
			case 20:
				rte_parse_portlist(argv[1], &portlist);
				rte_ether_aton(argv[3], &mac);
				cli_printf("   Show Portlist: %08x, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
						   portlist,
						   mac.addr_bytes[0],
						   mac.addr_bytes[1],
						   mac.addr_bytes[2],
						   mac.addr_bytes[3],
						   mac.addr_bytes[4],
						   mac.addr_bytes[5]);
				break;
			case 30:
				rte_parse_portlist(argv[1], &portlist);
				rte_ether_aton(argv[5], &mac);
				cli_printf("   Show Portlist: %08x vlan %d MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
						   portlist,
						   atoi(argv[3]),
						   mac.addr_bytes[0],
						   mac.addr_bytes[1],
						   mac.addr_bytes[2],
						   mac.addr_bytes[3],
						   mac.addr_bytes[4],
						   mac.addr_bytes[5]);
				break;
			case 40:
				rte_parse_portlist(argv[1], &portlist);
				rte_ether_aton("1234:4567:8901", &mac);
				cli_printf("   Show Portlist: %08x %s: ",
						   portlist, argv[2]);
				if (argv[2][0] == 'm')
					cli_printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
						   mac.addr_bytes[0],
						   mac.addr_bytes[1],
						   mac.addr_bytes[2],
						   mac.addr_bytes[3],
						   mac.addr_bytes[4],
						   mac.addr_bytes[5]);
				else
					cli_printf("%d\n", 101);
				break;
			default:
				cli_help_show_group("Show");
				return -1;
		}
		return 0;
	}

	static struct cli_tree my_tree[] = {
		c_dir("/data"),
	    c_file("pci",	pci_file, 	"display lspci information"),
	    c_dir("/bin"),
	    c_cmd("show",	show_cmd, 	"show mapping options"),
	    c_cmd("hello",	hello_cmd, 	"Hello-World!!"),
	    c_alias("h", 	"history", 	"display history commands"),
	    c_alias("ll", 	"ls -l", 	"long directory listing alias"),
	    c_end()
	};

Here is the cli_tree for this example, note it has a lot more commands. The show_cmd
or **show** command is located a number of lines down. The cli_tree creates in
the **/bin** directory a number of commands and the show command is one of these. The
show command has four different formats if you look at the **show_map[]** structure.

The user types one of these commands and cli_mapping() function attempts to locate the
correct entry in the list. You will also notice another structure called
**show_help**, which is an array of strings giving a cleaner and longer help
description of each of the commands.


Understanding the CLI system
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The command line interface is defined as a fake directory tree with executables,
directorys and files. The user uses shell like standard commands to move about
the directory and execute commands. The CLI is not a powerful as the
Bash shell, but has a number of similar concepts.

Our fake directory tree has a '/' or root directory which is created when
cli_create() is called along with the default sbin directory. The user starts out
at the root directory '/' and is allowed to cd to other directories, which could
contain more executables, aliases or directories. The max number of directory
levels is limited to the number of nodes given at startup.

The default directory tree starts out as just root (/) and a sbin directory.
Also it contains a file called copyright in root, which can be displayed
using the default 'more copyright' command.

A number of default commands are predefined in the /sbin directory and are
defined above. Other bin directories can be added to the system if needed,
but a limit of CLI_MAX_BINS is defined in the cli.h header file.
