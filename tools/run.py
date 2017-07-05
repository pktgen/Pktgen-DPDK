#! /usr/bin/env python
#
#   BSD LICENSE
#
#   Copyright(c) 2017 Intel Corporation. All rights reserved.
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     * Neither the name of Intel Corporation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import sys
import os
import getopt
import subprocess
import glob
from os.path import exists, abspath, dirname, basename
import imp

sdk = os.getenv("RTE_SDK", "")
target = os.getenv("RTE_TARGET", "x86_64-native-linux-gcc")
cfg_ext = '.cfg'
hugepage_path = "/sys/devices/system/node/node%d/hugepages/hugepages-2048kB/nr_hugepages"


def usage():
    '''Print usage information for the program'''
    argv0 = basename(sys.argv[0])
    print("""
Usage:
------

     %(argv0)s [options] config_file

where config_file is the one of the defined configuration files if no config
file is listed then the cfg/default.cfg file will be used.

Options:
    --help, --usage:
        Display usage information and quit

    -l, --list:
        Print a list of known configuration files


Examples:
---------

To display current list of configuration files:
        %(argv0)s --list

To run a config file:
        %(argv0)s default
    The configuration file name is always suffixed by .cfg as in default.cfg.
    The search location of the configuration files is .:./cfg

    """ % locals())  # replace items from local variables

def file_list(d, t):
    ''' Return list of configuration files '''
    fileiter = (os.path.join(root, f)
    	for root, _, files in os.walk(d)
        	for f in files)
    return (f for f in fileiter if os.path.splitext(f)[1] == t)

def show_configs():
    ''' Show run/init configuration files '''
    print("=== List of Configuration files ===")
    print("   %-16s - %s" % ("Name", "Description"))
    for fname in file_list('.', cfg_ext):
		base = os.path.splitext(os.path.basename(fname))[0]
		if base != "call_Uncrustify":
			try:
				f = open(fname)
			except:
				print("Error: unable to open file %s\n" % fname)
				sys.exit(1)
				
			desc = imp.load_source('cfg', '', f)
			f.close()
			os.unlink('c')

			if not desc.description:
				desc.description = ""
			print("   %-16s - %s" % (base, desc.description))
			desc.description = None
    
def load_cfg():
    print("Opening %s file\n" % cfg_file)
    if not os.path.exists(cfg_file):
        print("Run file %s does not exists\n" % cfg_file)
        sys.exit(1)
        
    try:
        f = open(cfg_file)
    except:
        print("Error: unable to open file %s\n" % cfg_file)
        sys.exit(1)
        
    global cfg
    cfg = imp.load_source('cfg', '', f)
    f.close()
    os.unlink('c')
    
def run_cfg():
    ''' Run the configuration '''
    args = []
    args.extend(["sudo"])
    args.extend(["./app/%s/pktgen" % target])
    
    for a in cfg.run['dpdk']:
        args.extend(a.split(' '))
    for a in cfg.run['blacklist']:
        args.extend(a.split(' '))
    args.extend(["--"])
    for a in cfg.run['pktgen']:
        args.extend(a.split(' '))
    for a in cfg.run['misc']:
        args.extend(a.split(' '))

    str = ""
    for a in args:
        str = str + "%s " % a
    print(str)
    subprocess.call(args)

    #subprocess.call(['echo', '%c[1;r' % 0x1b])
    #subprocess.call(['echo', '%c[99;H' % 0x1b])
    subprocess.call(['stty', 'sane'])

def num_sockets():
    sockets = 0
    for i in range(0, 8):
        if os.path.exists(hugepage_path % i):
            sockets = sockets + 1
    return sockets

def setup_cfg():
    ''' Setup the initial system '''

    nb_sockets = int(num_sockets())
    p = subprocess.Popen(['sysctl', '-n', 'vm.nr_hugepages'], stdout=subprocess.PIPE)
    nb_hugepages = int(p.communicate()[0]) / 2
    print("hugepages per socket %d" % nb_hugepages)

    subprocess.call(['sudo', 'rmmod', 'igb_uio'])
    subprocess.call(['sudo', 'modprobe', "uio"])
    subprocess.call(['sudo', 'insmod', "%s/%s/kmod/igb_uio.ko" % (sdk, target)])
    
    for i in range(0, nb_sockets):
        fn = (hugepage_path % i)
        print("Set %s to %d" % (fn, nb_hugepages))
        subprocess.call(['sudo', 'sh', '-c', 'eval', 'echo %s > %s' % (nb_hugepages, fn)])
        
    if os.path.exists("%s/usertools/dpdk-devbind.py" % sdk):
        nic_bind = "%s/usertools/dpdk-devbind.py" % sdk
    else:
        nic_bind = "%s/tools/dpdk_nic_bind.py" % sdk

    args = []
    args.extend(['sudo'])
    args.extend(['-E'])
    args.extend([nic_bind])
    
    for a in cfg.setup['opts']:
        args.extend(a.split(' '))
    for a in cfg.setup['devices']:
        args.extend(a.split(' '))
    
    print("Bind devices to DPDK:")
    print("   %s" % cfg.setup['devices'])
    subprocess.call(args)

def find_file(arg, t):
    ''' Find the first file matching the arg value '''
    fn = arg + cfg_ext
    for f in file_list('.', t):
        if os.path.basename(f) == fn:
            return f
    return None 

def parse_args():
    ''' Parse the command arguments '''
    global run_flag
    global cfg_file
    
    run_flag = True
    cfg_file = "./cfg/default" + cfg_ext
    
    if len(sys.argv) <= 1:
        usage()
        sys.exit(0)
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "huls",
                                   ["help", "usage", "list", "setup", ])
    except getopt.GetoptError as error:
        print(str(error))
        print("Run '%s --usage' for further information" % sys.argv[0])
        sys.exit(1)

    for opt, _ in opts:
        if opt == "--help" or opt == "-h" or opt == "--usage" or opt == "-u":
            usage()
            sys.exit(0)
        if opt == "--list" or opt == "-l":
            show_configs()
            sys.exit(0)
        if opt == "--setup" or opt == "-s":
            run_flag = False
    
    if not args or len(args) > 1:
        usage()
        sys.exit(1)

    fn = find_file(args[0], cfg_ext)
    if not fn:
        print("*** Config file '%s' not found" % args[0])
        show_configs()
        sys.exit(1)
    else:
        cfg_file = fn

def main():
    '''program main function'''
    if sdk == "":
        print("Set RTE_SDK environment variable")
        sys.exit(0)

    parse_args()

    load_cfg()

    if run_flag:
        run_cfg()
    else:
        setup_cfg()
        
if __name__ == "__main__":
    main()
