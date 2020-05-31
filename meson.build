project('pktgen', 'C',
    # Get version number from file.
    # Fallback to "more" for Windows compatibility.
    version: run_command(find_program('cat', 'more'),
        files('VERSION')).stdout().strip(),

	license: 'BSD',
	default_options: ['buildtype=release', 'default_library=static'],
	meson_version: '>= 0.47.1'
)

pktgen_conf = configuration_data()

#rte_sdk = ''
#cmd = run_command('sh', '-c', 'echo $RTE_SDK')
#if cmd.returncode() == 0
#	rte_sdk = cmd.stdout().strip()
#endif
#if rte_sdk == ''
#	# not found, do fallback
#	message('default rte_sdk to /usr/local')
#	rte_sdk='/usr/local'
#else
#	message('RTE_SDK=' + rte_sdk)
#endif
#
#rte_target = ''
#cmd = run_command('sh', '-c', 'echo $RTE_TARGET')
#if cmd.returncode() == 0
#	rte_target = cmd.stdout().strip()
#endif
#if rte_target == ''
#	# not found, do fallback
#	message('default rte_target to x86_64-linux-gnu')
#	rte_target='x86_64-linux-gnu'
#else
#	message('RTE_TARGET=' + rte_target)
#endif

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')

dpdk_libraries = []
dpdk_static_libraries = []
dpdk_drivers = []
dpdk_extra_ldflags = []
dpdk_app_link_libraries = []
pktgen_shared_libraries = []
pktgen_static_libraries = []
machine_args = ['-mavx']

pver = meson.project_version().split('.')
major_version = '@0@.@1@'.format(pver.get(0), pver.get(1))

add_project_arguments('-include', 'rte_config.h', language: 'c')
add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')

bsd_dep = dependency('libbsd', required: true)

dpdk_dep = dependency('libdpdk', required: true)

#dpdk_libdir = rte_sdk + dpdk_dep.get_pkgconfig_variable('libdir')
#dpdk_incs = rte_sdk + dpdk_dep.get_pkgconfig_variable('includedir')
#dpdk_libdir = dpdk_dep.get_pkgconfig_variable('libdir')
#dpdk_incs = dpdk_dep.get_pkgconfig_variable('includedir')
#message('Using dpdk_libdir = ' + dpdk_libdir)
#message('Using dpdk_incs   = ' + dpdk_incs)

#include_directories(dpdk_incs)
#add_global_link_arguments('-L'+dpdk_libdir, language: 'c')

subdir('lib')

subdir('app')

pkg = import('pkgconfig')
pkg.generate(name: meson.project_name(),
        filebase: 'lib' + meson.project_name().to_lower(),
        version: meson.project_version(),
#        libraries: dpdk_libraries,
#        libraries_private: dpdk_drivers + dpdk_static_libraries +
#                        ['-Wl,-Bdynamic'] + dpdk_extra_ldflags,
        description: '''Pktgen a Data Plane Development Kit (DPDK) application.
Note that CFLAGS might contain an -march flag higher than typical baseline.
This is required for a number of static inline functions in the public headers.''',
        subdirs: '.',
        extra_cflags: ['-include', 'rte_config.h'] + machine_args
)

# final output, list all the libs and drivers to be built
# this does not affect any part of the build, for information only.
output_message = '\n=================\nLibraries Enabled\n=================\n'
output_message += '\nlibs:\n\t'
output_count = 0
foreach lib:enabled_libs
        output_message += lib + ', '
        output_count += 1
        if output_count == 8
                output_message += '\n\t'
                output_count = 0
        endif
endforeach
message(output_message + '\n')
