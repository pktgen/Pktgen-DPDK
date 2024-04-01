#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2019-2024> Intel Corporation

# A simple script to help build Pktgen using meson/ninja tools.
# The script also creates an installed directory called ./usr in the top level directory.
# The install directory will contain all of the includes and libraries
# for external applications to build and link with Pktgen.
#
# using 'pktgen-build.sh help' or 'pktgen-build.sh -h' or 'pktgen-build.sh --help' to see help information.
#

buildtype="release"

currdir=`pwd`
script_dir=$(cd "${BASH_SOURCE[0]%/*}" & pwd -P)
export sdk_path="${PKTGEN_SDK:-${script_dir%/*}}"
export target_dir="${PKTGEN_TARGET:-usr/local}"
export build_dir="${PKTGEN_BUILD:-${currdir}/builddir}"
install_path="${PKTGEN_DESTDIR:-${currdir}}"

export lua_enabled="-Denable_lua=false"

configure="setup"

if [[ "${build_dir}" = /* ]]; then
	# absolute path to build dir. Do not prepend workdir.
	build_path=$build_dir
else
	build_path=${currdir}/$build_dir
fi

if [[ ! "${install_path}" = /* ]]; then
	# relative path for install path detected
	# prepend with currdir
	install_path=${currdir}/${install_path}
fi

if [[ "${target_dir}" = .* ]]; then
	echo "target_dir starts with . or .. if different install prefix required then use PKTGEN_DESTDIR instead"
	exit 1
fi
if [[ "${target_dir}" = /* ]]; then
	echo "target_dir absolute path detected removing leading '/'"
	export target_dir=${target_dir##/}
fi
target_path=${install_path%/}/${target_dir%/}

echo ">>  SDK Path          : "$sdk_path
echo ">>  Install Path      : "$install_path
echo ">>  Build Directory   : "$build_dir
echo ">>  Target Directory  : "$target_dir
echo ">>  Build Path        : "$build_path
echo ">>  Target Path       : "$target_path
echo ""

function dump_options() {
	echo " Build and install values:"
	echo "   lua_enabled       : "$lua_enabled
	echo ""
}

function run_meson() {
	btype="-Dbuildtype="$buildtype

    echo "meson $configure $btype $lua_enabled $build_dir"
	if ! meson $configure $btype $lua_enabled $build_dir; then
        echo "*** ERROR: meson $configure $btype $lua_enabled $build_dir"
        configure=""
        return 1
    fi

    configure=""
    return 0
}

function ninja_build() {
	echo ">>> Ninja build in '"$build_path"' buildtype="$buildtype

	if [[ -d $build_path ]] || [[ -f $build_path/build.ninja ]]; then
		# add reconfigure command if meson dir already exists
		configure="configure"
		# sdk_dir must be empty if we're reconfiguring
		sdk_dir=""
	fi
	if ! run_meson; then
        return 1
    fi

	ninja -C $build_path

	if [[ $? -ne 0 ]]; then
		return 1;
	fi

	return 0
}

function ninja_build_docs() {
	echo ">>> Ninja build documents in '"$build_path"'"

	if [[ ! -d $build_path ]] || [[ ! -f $build_path/build.ninja ]]; then
		if ! run_meson; then
            return 1
        fi
	fi

	ninja -C $build_path doc

	if [[ $? -ne 0 ]]; then
		return 1;
	fi
	return 0
}

ninja_install() {
	echo ">>> Ninja install to '"$target_path"'"

	DESTDIR=$install_path ninja -C $build_path install

	if [[ $? -ne 0 ]]; then
		echo "*** Install failed!!"
		return 1;
	fi
	return 0
}

ninja_uninstall() {
	echo ">>> Ninja uninstall to '"$target_path"'"

	DESTDIR=$install_path ninja -C $build_path uninstall

	if [[ $? -ne 0 ]]; then
		echo "*** Uninstall failed!!"
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
	echo "  buildlua    - same as 'make build' except enable Lua build"
	echo "  debug       - turn off optimization, may need to do 'clean' then 'debug' the first time"
	echo "  debugopt    - turn optimization on with -O2, may need to do 'clean' then 'debugopt' the first time"
	echo "  clean       - remove the following directory: "$build_path
	echo "  install     - install the includes/libraries into "$target_path" directory"
	echo "  uninstall   - uninstall the includes/libraries into "$target_path" directory"
	echo "  docs        - create the document files"
	echo ""
	echo " Build and install environment variables:"
	echo "  PKTGEN_INSTALL_PATH - The install path, defaults to Pktgen SDK directory"
	echo "  PKTGEN_TARGET       - The target directory appended to install path, defaults to 'usr'"
	echo "  PKTGEN_BUILD        - The build directory appended to install path, default to 'builddir'"
	echo "  PKTGEN_DESTDIR      - The install destination directory"
	echo ""
	dump_options
	exit
}

for cmd in "$@"
do
	case "$cmd" in
	'help' | '-h' | '--help')
		usage
		;;

	'build')
		dump_options
		ninja_build && ninja_install
		;;

	'buildlua')
		lua_enabled="-Denable_lua=true"
		dump_options
		ninja_build && ninja_install
		;;

	'debuglua')
		lua_enabled="-Denable_lua=true"
		buildtype="debug"
		dump_options
		ninja_build && ninja_install
		;;

	'debug')
		buildtype="debug"
		dump_options
		ninja_build && ninja_install
		;;

	'debugopt')
		echo ">>> Debug Optimized build in '"$build_path"' and '"$target_path"'"
		buildtype="debugoptimized"
		dump_options
		ninja_build && ninja_install
		;;

	'clean')
		dump_options
		echo "*** Removing '"$build_path"' directory"
		rm -fr $build_path
		;;

	'install')
		dump_options
		echo ">>> Install the includes/libraries into '"$target_path"' directory"
		ninja_install
		;;

	'uninstall')
		dump_options
		echo ">>> Uninstall the includes/libraries from '"$target_path"' directory"
		ninja_uninstall
		;;

	'docs')
		dump_options
		echo ">>> Create the documents '"$build_path"' directory"
		ninja_build_docs
		;;

	*)
		if [[ $# -gt 0 ]]; then
			usage
		else
			echo ">>> Build and install Pktgen"
			dump_options
			ninja_build && ninja_install
		fi
		;;
	esac
done
