project('pktgen', 'C',
	version: '19.05.0-rc0',
	license: 'BSD',
	default_options: ['buildtype=release', 'default_library=static'],
	meson_version: '>= 0.47.1'
)

cmd = run_command('sh', '-c', 'echo $RTE_SDK')
if cmd.returncode() == 0
	rte_sdk = cmd.stdout().strip()
	message('RTE_SDK=' + rte_sdk)
else
  # not found, do fallback
  error('Please set RTE_SDK env')
endif

cmd = run_command('sh', '-c', 'echo $RTE_TARGET')
if cmd.returncode() == 0
	rte_target = cmd.stdout().strip()
	message('RTE_TARGET=' + rte_target)
else
  # not found, do fallback
  error('Please set RTE_TARGET')
endif

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')

pktgen_conf = configuration_data()
dpdk_libraries = []
dpdk_static_libraries = []
dpdk_drivers = []
dpdk_extra_ldflags = []
dpdk_app_link_libraries = []
global_inc = []
dpdk_dep = []
pmdk_shared_libraries = []
pmdk_static_libraries = []

pver = meson.project_version().split('.')
major_version = '@0@.@1@'.format(pver.get(0), pver.get(1))

add_project_arguments('-D_GNU_SOURCE', '-O3', '-g', language: 'c')

dpdk_incs = include_directories(rte_sdk + '/' + rte_target + '/include')
dpdk_libdir = rte_sdk + '/' + rte_target + '/lib'
if get_option('lib_dir') != ''
	if run_command('[', '-d', dpdk_libdir + '/' + get_option('lib_dir'), ']').returncode() == 0
		dpdk_libdir = dpdk_libdir + '/' + get_option('lib_dir')
	endif
endif

message('Using dpkd_libdir = ' + dpdk_libdir)

subdir('lib')
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
        subdirs: [get_option('include_subdir_arch'), '.'],
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

output_message = '\n===============\nDrivers Enabled\n===============\n'
foreach class:driver_classes
        class_drivers = get_variable(class + '_drivers')
        output_message += '\n' + class + ':\n\t'
        output_count = 0
        foreach drv:class_drivers
                output_message += drv + ', '
                output_count += 1
                if output_count == 8
                        output_message += '\n\t'
                        output_count = 0
                endif
        endforeach
endforeach
message(output_message + '\n')
