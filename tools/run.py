#! /usr/bin/env python
#
#   Copyright(c) 2019 Intel Corporation. All rights reserved.
#
#  SPDX-License-Identifier: BSD-3-Clause
#

import sys
import os
import getopt
import subprocess
import glob
import imp

def usage():
	'''Print usage information for the program'''
	argv0 = basename(sys.argv[0])
	print("""
Usage:
------
  %(argv0)s [options] [config_name]

  Where config_name is the one of the defined configuration files if no config
  file is listed then the ./default.cfg file will be used. If a cfg directory
  is located in the current directory then it will be searched for a match.

  The config_name is the name of the file without path and .cfg extension.

Options:
--------
    -h, --help, -u, --usage:
	    Display usage information and quit

    -l, --list:
	    Print a list of known configuration files

	-n, --norun
        Just create the command line and outpuyt the line with no-running.

    -v, --verbose
	    Print out more information

Examples:
---------
  To display current list of configuration files:
	%(argv0)s --list

  To run a config file:
	%(argv0)s default

  The configuration file name is always suffixed by .cfg as in default.cfg.
  The search location of the configuration files is .:./cfg

	""" % locals())  # replace items from local variables
	sys.exit(0)

def err_exit(str):
	''' print the error string and exit '''
	print(str)
	sys.exit(1)

def find_file(fn, t):
	''' Find the first file matching the arg value '''
        f = os.path.splitext(fn)
        if f[1] == t:
	    fn = f[0]
	for f in file_list('cfg', t):
                b = os.path.basename(f)
		if os.path.splitext(b)[0] == fn:
			return f
	return None

def mk_tuple(lst, s):
	''' Convert a string to a tuple if needed '''
	t = {}

	if type(lst[s]) != tuple:
		if verbose:
			print('Not a Tuple', type(lst[s]), lst[s])
		t[s] = tuple([lst[s],])
	else:
		if verbose:
			print('A tuple', type(lst[s]), lst[s])
		t[s] = lst[s]

	if verbose:
		print('New t[s]', type(t[s]), t[s])

	return t[s]

def add_ld_options(s, arg_list):
	''' Append LD_LIBRARY_PATH option to arg list '''
	if s in cfg.run:            
            str = 'LD_LIBRARY_PATH=.'
            for a in mk_tuple(cfg.run, s):
                _p = a % globals()
                str = str + ':' + _p
            arg_list.append(str)

def add_run_options(s, arg_list, p):
	''' Append options to arg list '''
	if s in cfg.run:
            for a in mk_tuple(cfg.run, s):
                if p is not None:
                    arg_list.append(p)

                _p = a % globals()
                arg_list.append(_p)

def add_setup_options(s, arg_list):
	''' Append options to arg list '''
	if s in cfg.setup:
		for a in mk_tuple(cfg.setup, s):
			arg_list.extend(a.split(' '))

def file_list(directory, file_extension):
	''' Return list of configuration files '''
	fileiter = (os.path.join(root, f)
		for root, _, files in os.walk(directory)
			for f in files)
	return (f for f in fileiter if os.path.splitext(f)[1] == file_extension)

def load_cfg(fname):
	''' Load the configuration or .cfg file as a python data file '''

	if not os.path.exists(fname):
		err_exit("Config file %s does not exists\n" % fname)

	try:
		configuration_file = open(fname)
	except:
		err_exit("Error: unable to open file %s\n" % fname)

	global cfg
	cfg = imp.load_source('cfg', fname, configuration_file)

        # The command above will create a file called <filename>c and needs to be removed.
        # The file is the compiled python code from the config file.
        os.unlink(fname + 'c');

	configuration_file.close()

	return cfg

def show_configs():
	''' Show configuration files '''

	print("Configurations:")
	print("   %-16s - %s" % ("Name", "Description"))
	print("   %-16s   %s" % ("----", "-----------"))

	for fname in file_list('cfg', '.cfg'):
		base = os.path.splitext(os.path.basename(fname))[0]

		try:
			cfg = load_cfg(fname)

			if not cfg.description:
				cfg.description = ""
			print("   %-16s - %s" % (base, cfg.description))
		except NameError:
			sys.stderr.write("We were unable to load the module " + fname + \
			" If you do not plan to use this module you can safely ignore this " \
			"message.\n")
		finally:
			# reset the descriptoin to empty, for next loop/file
			cfg.description = ""

	sys.exit(0)

def run_cfg(cfg_file):
	''' Run the configuration in the .cfg file '''

	cfg = load_cfg(cfg_file)

	args = []

	add_run_options('exec', args, None)

	add_ld_options('ld_path', args)

	if not 'app_path' in cfg.run:
		err_exit("'app_path' variable is missing from cfg.run in config file")

	if not 'app_name' in cfg.run:
		err_exit("'app_name' variable is missing from cfg.run in config file")

	# convert the cfg.run['app_name'] into a global variable used in
	# the creation of the applicaiton/path. app_name must be a global variable.
	global app_name
	app_name = cfg.run['app_name']

	# Try all of the different path versions till we find one.
	fname = None
	for app in cfg.run['app_path']:
		fn = app % globals()
		print("   Trying %s" % fn)
		if os.path.exists(fn):
			fname = fn
			if verbose:
				print("Found %s" % fn)
			break

	if not fname:
		err_exit("Error: Unable to locate application %s" % cfg.run['app_name'])

	args.extend([fname])

	add_run_options('cores', args, '-l')
	add_run_options('nrank', args, '-n')
	add_run_options('proc', args, '--proc-type')
	add_run_options('log', args, '--log-level')
	add_run_options('prefix', args, '--file-prefix')
	add_run_options('blacklist', args, '-b')
	add_run_options('whitelist', args, '-w')
	add_run_options('vdev', args, '--vdev')
	add_run_options('plugin', args, '-d')
	args.extend(["--"])
	add_run_options('opts', args, None)
	add_run_options('map', args, '-m')
	add_run_options('pcap', args, '-s')
	add_run_options('theme', args, '-f')
	add_run_options('loadfile', args, '-f')
	add_run_options('logfile', args, '-l')

	# Convert the args list to a single string with spaces.
	str = ""
	for a in args:
		str = str + "%s " % a

	# Output the command line
	print(str)
	if norun:
		return

	if verbose:
		print("Command line as a set:")
		print(args)

	subprocess.call(args)

	subprocess.call(['stty', 'sane'])

def num_sockets(hpath):
	''' Count the number of sockets in the system '''

	sockets = 0
	for i in range(0, 8):
		if os.path.exists(hpath % i):
			sockets = sockets + 1

	return sockets

def setup_cfg(cfg_file):
	''' Setup the system by adding modules and ports to dpdk control '''

	cfg = load_cfg(cfg_file)

	print("Setup DPDK to run '%s' application from %s file" %
		   (cfg.run['app_name'], cfg_file))

	sys_node = '/sys/devices/system/node/node%d/hugepages'
	hugepage_path = sys_node + '/hugepages-2048kB/nr_hugepages'

	# calculate the number of sockets in the system.
	nb_sockets = int(num_sockets(hugepage_path))
	if nb_sockets == 0:
		nb_sockets = 1

	p = subprocess.Popen(['sysctl', '-n', 'vm.nr_hugepages'],
						 stdout=subprocess.PIPE)

	# split the number of hugepages between the sockets
	nb_hugepages = int(p.communicate()[0]) / nb_sockets

	if verbose:
		print("  Hugepages per socket %d" % nb_hugepages)

	if verbose:
		print("  modprobe the 'uio' required module")

        if 'uio' in cfg.setup:
            u = cfg.setup['uio']

            if u == 'igb_uio':
                subprocess.call(['sudo', 'modprobe', "uio"])

                if verbose:
                        print("  Remove %s if already installed" % u)

                ret = subprocess.call(['sudo', 'rmmod', u])
                if ret > 0:
                        print("  Remove of %s, displayed an error ignore it" % u)

                uio = ("%s/%s/kmod/%s.ko" % (sdk, target, u))
                if verbose:
                        print("  insmode the %s module" % uio)
                subprocess.call(['sudo', 'insmod', uio])

            if u == 'vfio-pci':
                ret = subprocess.call(['sudo', 'rmmod', u])
                if ret > 0:
                        print("  Remove of %s, displayed an error ignore it" % u)

                if verbose:
                        print("  modprobe the %s module" % u)
                subprocess.call(['sudo', 'modprobe', u])

            if u == 'uio_pci_generic':
                ret = subprocess.call(['sudo', 'rmmod', u])
                if ret > 0:
                        print("  Remove of %s, displayed an error ignore it" % u)

                if verbose:
                        print("  insmode the %s module" % u)
                subprocess.call(['sudo', 'modprobe', u])
        else:
            if u == 'vfio-pci':
                ret = subprocess.call(['sudo', 'rmmod', u])
                if ret > 0:
                        print("  Remove of %s, displayed an error ignore it" % u)

                if verbose:
                        print("  modprobe the %s module" % u)
                subprocess.call(['sudo', 'modprobe', u])

	for i in range(0, nb_sockets):
		fn = (hugepage_path % i)
		if verbose:
			print("  Set %d socket to %d hugepages" % (i, nb_hugepages))
		subprocess.call(['sudo', '-E', 'sh', '-c', 'eval',
					 'echo %s > %s' % (nb_hugepages, fn)])

	# locate the binding tool
	if os.path.exists("%s/usertools/dpdk-devbind.py" % sdk):
		script = "%s/usertools/dpdk-devbind.py" % sdk
	elif os.path.exists("%s/tools/dpdk_nic_bind.py" % sdk):
		script = "%s/tools/dpdk_nic_bind.py" % sdk
	else:
		err_exit("Error: Failed to find dpdk-devbind.py or dpdk_nic_bind.py")

	# build up the system command line to be executed
	args = []
	add_setup_options('exec', args)

	args.extend([script])

        args.append('-b')
        args.append(cfg.setup['uio'])

	add_setup_options('devices', args)

	if verbose:
		print("  Bind following devices to DPDK:")
		for a in cfg.setup['devices']:
			print("		%s" % a)
		print(args)

	subprocess.call(args)

def parse_args():
	''' Parse the command arguments '''

	global run_flag, verbose, norun

	run_flag = True
	verbose = False
	norun = False

	cfg_file = "./cfg/default.cfg"

	if len(sys.argv) <= 1:
		print("*** Pick one of the following config files\n")
		show_configs()

	try:
		opts, args = getopt.getopt(sys.argv[1:], "hulsvn",
				["help", "usage", "list", "setup", "verbose", "norun", ])

	except getopt.GetoptError as error:
		print(str(error))
		usage()

	for opt, _ in opts:
		if opt == "--help" or opt == "-h":
			usage()
		if opt == "--usage" or opt == "-u":
			usage()
		if opt == "--list" or opt == "-l":
			show_configs()
		if opt == "--setup" or opt == "-s":
			run_flag = False
		if opt == "--verbose" or opt == "-v":
			verbose = True
		if opt == "--norun" or opt == "-n":
			norun = True

	if not args or len(args) > 1:
		usage()

	fn = find_file(args[0], '.cfg')
	if not fn:
                f = args[0]
                if os.path.splitext(args[0])[1] != '.cfg':
                    f = args[0] + '.cfg'
		print("*** Config file '%s' not found" % f)
                print("    Make sure you are running this command in pktgen top directory")
                print("    e.g. cd Pktgen-DPDK; ./tools/run.py default")
		show_configs()
	else:
		cfg_file = fn

	return cfg_file

def main():
	'''program main function'''

	global sdk, target, pmdk

	sdk = os.getenv('RTE_SDK')
	if sdk == None:
		err_exit("Set RTE_SDK environment variable or use 'sudo -E ...'")

	pmdk = os.getenv('PMDK_SDK')
	if pmdk == None:
		pmdk = 'pmdk-not-set';

	target = os.getenv('RTE_TARGET')
	if target == None:
		err_exit("Set the RTE_TARET environment variable or use 'sudo -E ...'")

	print(">>> sdk '%s', target '%s'" % (sdk, target))

	cfg_file = parse_args()

	if run_flag:
		run_cfg(cfg_file)
	else:
		setup_cfg(cfg_file)

if __name__ == "__main__":
	main()
