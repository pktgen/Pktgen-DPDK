# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2020 Intel Corporation

libraries = ['common', 'utils', 'vec', 'plugin', 'cli', 'lua']

enabled_libs = []
foreach l:libraries
	build = true
	name = l
	sources = []
	includes = []
	cflags = []
	objs = [] # other object files to link against, used e.g. for
	          # instruction-set optimized versions of code

	# use "deps" for internal Pktgen dependencies, and "ext_deps" for
	# external package/library requirements
	ext_deps = [dpdk_dep]
	deps = []

	dir_name = l
	message('Process ' + dir_name)
	subdir(dir_name)

	if build
		enabled_libs += name
		libname = name
		includes += include_directories(dir_name)

		message('    Sources in '+ libname)
		static_deps = ext_deps
		foreach d:deps
			if not is_variable('static_pktgen_' + d)
				error('Missing dependency ' + d +
					' for library ' + libname)
			else
				message('    === Found dependency ' + d)
			endif
			static_deps += [get_variable('static_pktgen_' + d)]
		endforeach

		# first build static lib
		static_lib = static_library(libname,
				sources,
				objects: objs,
				c_args: cflags,
				dependencies: static_deps,
				include_directories: includes,
				install: true)
		static_dep = declare_dependency(link_with: static_lib,
				include_directories: includes,
				dependencies: static_deps)

		set_variable('static_pktgen_' + libname, static_dep)
	endif # if build
endforeach
