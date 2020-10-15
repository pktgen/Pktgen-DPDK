#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2019-2020 Intel Corporation

# A simple script to help build Pktgen using meson/ninja tools.
# The script also creates an installed directory called ./usr in the top level directory.
# The install directory will contain all of the includes and libraries
# for external applications to build and link with Pktgen.
#
# using 'pktgen-build.sh help' or 'pktgen-build.sh -h' or 'pktgen-build.sh --help' to see help information.
#

buildtype="release"
currdir=`pwd`

export sdk_dir="${PKTGEN_SDK:-$currdir}"
export target_dir="${PKTGEN_TARGET:-usr}"
export build_dir="${PKTGEN_BUILD_DIR:-Builddir}"
export lua_enabled="-Denable_lua=false"
export gui_enabled="-Denable_gui=false"

if [[ -n ${enable_lua} ]]; then
	export lua_enabled="-Denable_lua=true"
fi
if [[ -n ${enable_gui} ]]; then
	export gui_enabled="-Denable_gui=true"
fi

build_path=$sdk_dir/$build_dir
target_path=$sdk_dir/$target_dir

echo ">>> lua_enabled      : '"$lua_enabled"'"
echo ">>> gui_enabled      : '"$gui_enabled"'"
echo ">>> SDK Directory    : '"$sdk_dir"'"
echo ">>> Build Directory  : '"$build_path"'"
echo ">>> Target Directory : '"$target_path"'"
echo ""

function run_meson() {
	btype="-Dbuildtype="$buildtype

	echo meson $btype $lua_enabled $gui_enabled $build_dir
	meson $btype $lua_enabled $gui_enabled $build_dir
}

function ninja_build() {
	echo ">>> Ninja build in '"$build_path"' buildtype='"$buildtype"'"

	if [[ ! -d $$build_path ]]; then
		run_meson
	fi

	ninja -C $build_path

	if [[ $? -ne 0 ]]; then
		return 1;
	fi
	return 0
}

function ninja_build_docs() {
	echo ">>> Ninja build documents in '"$build_path"'"

	if [[ ! -d $build_path ]]; then
		run_meson
	fi

	ninja -C $build_path doc

	if [[ $? -ne 0 ]]; then
		return 1;
	fi
	return 0
}

ninja_install() {
	echo ">>> Ninja install to '"$target_path"'"

	DESTDIR=$sdk_dir ninja -C $build_path install > /dev/null

	if [[ $? -ne 0 ]]; then
		echo "*** Install failed!!"
		return 1;
	fi
	return 0
}

usage() {
	echo " Usage: Build Pktgen using Meson/Ninja tools"
	echo "  ** Must be in the top level directory for Pktgen"
	echo "     This tool is in tools/pktgen-build.sh, but use 'make' which calls this script"
	echo "     Use 'make' to build Pktgen as it allows for multiple targets i.e. 'make clean debug'"
	echo ""
	echo "  Command Options:"
	echo "  <no args>   - create the '"$build_dir"' directory if not present and compile Pktgen"
	echo "                If the '"$build_dir"' directory exists it will use ninja to build Pktgen without"
	echo "                running meson unless one of the meson.build files were changed"
	echo "  build       - same as 'make' with no arguments"
	echo "  buildgui    - same as 'make build' except enable gui build"
	echo "  buildlua    - same as 'make build' except enable Lua build"
	echo "  debug       - turn off optimization, may need to do 'clean' then 'debug' the first time"
	echo "  debugopt    - turn optimization on with -O2, may need to do 'clean' then 'debugopt' the first time"
	echo "  clean       - remove the '"$build_dir"' and '"$target_dir"' directories then exit"
	echo "  dist-clean  - remove the '"$build_dir"' directory leaving '"$target_dir"'"
	echo "  install     - install the includes/libraries into '"$target_dir"' directory"
	echo "  docs        - create the document files"
	echo "--- Make targets:"
	exit
}

for cmd in "$@"
do
	case "$cmd" in
	'help' | '-h' | '--help')
		usage
		;;

	'build')
		ninja_build && ninja_install
		;;

	'buildgui')
		gui_enabled="-Denable_gui=true"
		lua_enabled="-Denable_lua=true"
		ninja_build && ninja_install
		;;

	'buildlua')
		lua_enabled="-Denable_lua=true"
		ninja_build && ninja_install
		;;

	'debuglua')
		lua_enabled="-Denable_lua=true"
		buildtype="debug"
		ninja_build && ninja_install
		;;

	'debug')
		buildtype="debug"
		ninja_build && ninja_install
		;;

	'debugopt')
		echo ">>> Debug Optimized build in '"$build_path"' and '"$target_path"'"
		buildtype="debugoptimized"
		ninja_build && ninja_install
		;;

	'clean')
		echo "*** Removing '"$build_path"' directory"
		echo "*** Removing '"$target_path"' directory"
		rm -fr $build_dir $target_dir
		;;

	'dist-clean')
		echo ">>> Removing '"$build_path"' directory"
		rm -fr $build_path
		;;

	'install')
		echo ">>> Install the includes/libraries into '"$target_path"' directory"
		ninja_install
		;;

	'docs')
		echo ">>> Create the documents '"$build_path"' directory"
		ninja_build_docs
		;;

	*)
		if [[ $# -gt 0 ]]; then
			usage
		else
			echo ">>> Build and install Pktgen"
			ninja_build && ninja_install
		fi
		;;
	esac
done
