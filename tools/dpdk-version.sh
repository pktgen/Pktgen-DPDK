#!/usr/bin/env sh
#
#   Copyright(c) 2019 Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Locate the rte_version.h file and parse out the version strings.
#
# If RTE_SDK is set, then it is used to locate the file. If the
# RTE_SDK variable is not set then the current working directory
# is used.
#
# See usage below for more details.
#

fname=rte_version.h

usage() {
	cat <<EOM
  Usage: dpdk-version.sh

  Locate the rte_version.h file and parse out the version information.

  If RTE_SDK is set, then it is used to locate the file. If the
  RTE_SDK variable is not set then the current working directory
  is used.

  The default output of the version is 'DPDK Version: YY.MM.MINOR'
	e.g. 'dpdk-version.sh' gives 'DPDK Version: 17.08.0'

  Options: Only one option can be used at a time, the only exception
           is the '-v' option which must be first followed by any
           other option.

  '-h' prints usage messages

  '-v' print out the location of the file used, must be first option
    e.g. 'dpdk-version.sh -v -l' gives file path plus '-l' output.

  '-l' prints out the full version string
	e.g. 'dpdk-version.sh -l' gives 'DPDK-17.08.0-rc0'

  '-s' prints a shorter version string
	e.g. 'dpdk-version.sh -s' gives '17.08.0'

  '-yy' prints the year number
  '-mm' prints the month number
  '-mi' prints the minor number
  '-rc' prints the suffix string

  '-m' prints out information is comma delimited format.
	e.g. 'dpdk-version.sh -m' give 'DPDK,17,08,0,-rc0'

  '-p' prints out a Python readable format of the version.
	e.g. 'dpdk-version.sh -m' gives

	version = {
		'prefix': 'DPDK',
		'year': '17',
		'month': '08',
		'minor': '0',
		'suffix': '-rc'
	}
EOM
	exit 1
}

if [ -z ${RTE_INCLUDE} ] ; then
	if [ -z ${RTE_SDK} ] ; then
		echo "*** RTE_SDK is not set, using "${PWD}
		sdk=${PWD}
	else
		sdk=${RTE_SDK}
	fi

	fpath=${sdk}/lib/librte_eal/common/include/${fname}
else
	fpath=${RTE_INCLUDE}/${fname}
fi

if [ ! -e ${fpath} ]; then
	echo "File not found @ "${fpath}
	usage
fi

PREFIX=`grep "define RTE_VER_PREFIX" ${fpath} | cut -d ' ' -f 3 | sed -e 's/\"//g'`
SUFFIX=`grep "define RTE_VER_SUFFIX" ${fpath} | cut -d ' ' -f 3 | sed -e 's/\"//g'`

YY=`grep "define RTE_VER_YEAR" ${fpath} | cut -d ' ' -f 3`
MM=`grep "define RTE_VER_MONTH" ${fpath} | cut -d ' ' -f 3`
MINOR=`grep "define RTE_VER_MINOR" ${fpath} | cut -d ' ' -f 3`

print_path="no"
if [ "$1" = "-v" ] ; then
	print_path="yes"
	shift
fi

if [ "$1" = "-h" ]; then
	usage
	print_path="no"
elif [ "$1" = "-l" ]; then
	echo ${PREFIX}-${YY}.${MM}.${MINOR}${SUFFIX}
elif [ "$1" = '-s' ]; then
	echo ${YY}.${MM}.${MINOR}
elif [ "$1" = "-yy" ]; then
	echo ${YY}
elif [ "$1" = "-mm" ]; then
	echo ${MM}
elif [ "$1" = "-mi" ]; then
	echo ${MINOR}
elif [ "$1" = "-rc" ]; then
	echo ${SUFFIX}
elif [ "$1" = "-p" ]; then
	echo "dpdk_version = {"
	if [ ${print_path} = "yes" ] ; then
		echo "    'path': '"${fpath}"',"
		print_path="no"
	fi
	echo "    'prefix': '"${PREFIX}"',"
	echo "    'year': '"${YY}"',"
	echo "    'month': '"${MM}"',"
	echo "    'minor': '"${MINOR}"',"
	echo "    'suffix': '"${SUFFIX}"'"
	echo "}"
elif [ "$1" = "-m" ]; then
	echo ""${PREFIX}","${YY}","${MM}","${MINOR}","${SUFFIX}""
else
	echo "DPDK Version: "${YY}.${MM}.${MINOR}
fi
if [ ${print_path} = "yes" ] ; then
	echo "File: "${fpath}
fi
