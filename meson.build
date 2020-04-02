project('pktgen', 'C',
	version: '19.05.0-rc0',
	license: 'BSD',
	default_options: ['buildtype=release', 'default_library=static'],
	meson_version: '>= 0.47.1'
)

pktgen_conf = configuration_data()

rte_sdk = ''
cmd = run_command('sh', '-c', 'echo $RTE_SDK')
if cmd.returncode() == 0
	rte_sdk = cmd.stdout().strip()
endif
if rte_sdk == ''
	# not found, do fallback
	message('default rte_sdk to /usr/local')
	rte_sdk='/usr/local'
else
	message('RTE_SDK=' + rte_sdk)
endif

rte_target = ''
cmd = run_command('sh', '-c', 'echo $RTE_TARGET')
if cmd.returncode() == 0
	rte_target = cmd.stdout().strip()
endif
if rte_target == ''
	# not found, do fallback
	message('default rte_target to x86_64-linux-gnu')
	rte_target='x86_64-linux-gnu'
else
	message('RTE_TARGET=' + rte_target)
endif

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')

dpdk_libraries = []
dpdk_static_libraries = []
dpdk_drivers = []
dpdk_extra_ldflags = []
dpdk_app_link_libraries = []
dpdk_dep = []
pktgen_shared_libraries = []
pktgen_static_libraries = []
machine_args = ['-mavx']

pver = meson.project_version().split('.')
major_version = '@0@.@1@'.format(pver.get(0), pver.get(1))

add_project_arguments('-include', 'rte_config.h', language: 'c')
add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', '-O3', '-g', language: 'c')

dpdk_incs = include_directories(rte_sdk + '/include')
dpdk_libdir = rte_sdk + '/lib/' + rte_target
#if get_option('lib_dir') != ''
#	if run_command('[', '-d', dpdk_libdir + '/' + get_option('lib_dir'), ']').returncode() == 0
#		dpdk_libdir = dpdk_libdir + '/' + get_option('lib_dir')
#	endif
#endif

message('Using dpdk_libdir = ' + dpdk_libdir)

subdir('lib')

message('Using dpdk_libdir = ' + dpdk_libdir)
subdir('app')

pkg = import('pkgconfig')
pkg.generate(name: meson.project_name(),
        filebase: 'lib' + meson.project_name().to_lower(),
        version: meson.project_version(),
        libraries: dpdk_libraries,
        libraries_private: dpdk_drivers + dpdk_static_libraries +
                        ['-Wl,-Bdynamic'] + dpdk_extra_ldflags,
        description: '''The Data Plane Development Kit (DPDK).
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
