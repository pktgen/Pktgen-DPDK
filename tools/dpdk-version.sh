#!/bin/bash
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
# Locate the rte_version.h file and parse out the version strings.
#
# If RTE_SDK is set, then it is used to locate the file. If the
# RTE_SDK variable is not set then the current working directory
# is used.
#
# See usage below for more details.
#

fname=rte_version.h

function usage() {
	cat <<EOM
  Usage: dpdk-version.sh

  Locate the rte_version.h file and parse out the version information.

  If RTE_SDK is set, then it is used to locate the file. If the
  RTE_SDK variable is not set then the current working directory
  is used.

  The default output of the version is 'DPDK Version: YY.MM.MINOR'
	e.g. 'dpdk-version.sh' gives 'DPDK Version: 17.08.0'

  Options: Only one option can be used at a time.

  '-h' prints usage messages

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

if [ -z ${RTE_SDK} ] ; then
	echo "*** RTE_SDK is not set, using "${PWD}
	sdk=${PWD}
else
	sdk=${RTE_SDK}
fi

fpath=${sdk}/lib/librte_eal/common/include/${fname}

if [ ! -e ${fpath} ]; then
	echo "File not found @ "${fpath}
	usage
fi

PREFIX=`grep "define RTE_VER_PREFIX" ${fpath} | cut -d ' ' -f 3 | sed -e 's/\"//g'`
SUFFIX=`grep "define RTE_VER_SUFFIX" ${fpath} | cut -d ' ' -f 3 | sed -e 's/\"//g'`

YY=`grep "define RTE_VER_YEAR" ${fpath} | cut -d ' ' -f 3`
MM=`grep "define RTE_VER_MONTH" ${fpath} | cut -d ' ' -f 3`
MINOR=`grep "define RTE_VER_MINOR" ${fpath} | cut -d ' ' -f 3`

if [ "$1" = "-h" ]; then
	usage
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
