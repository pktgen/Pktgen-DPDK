project('pktgen', 'C',
   	version: run_command(find_program('cat', 'more'), files('VERSION')).stdout().strip(),
	license: 'BSD',
	default_options: ['buildtype=release', 'default_library=static'],
	meson_version: '>= 0.47.1'
)

pktgen_conf = configuration_data()

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
cflags = []
global_deps = []

pver = meson.project_version().split('.')
major_version = '@0@.@1@'.format(pver.get(0), pver.get(1))

add_project_arguments('-include', 'rte_config.h', language: 'c')
add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')

global_deps += dependency('libbsd', required: true)
global_deps += dependency('libdpdk', required: true)

if get_option('enable_lua')
    message('************** LUA is enabled')

    add_project_arguments('-DLUA_ENABLED', language: 'c')

    dep = dependency('lua5.3', required: false)
    if not dep.found()
        dep = cc.find_library('lua5.3', required: true)
    endif
    global_deps += dep
endif

if get_option('enable_gui')
    message('************** GUI is enabled')

    add_project_arguments('-DGUI_ENABLED', language: 'c')
    add_project_arguments('-Wno-deprecated-declarations', language: 'c')
    add_project_arguments('-DGUI', language: 'c')
    global_deps += [dependency('glib-2.0', required: true), dependency('gtk+-2.0', required: true)]
endif

global_deps += cc.find_library('numa', required: true)

dep = dependency('libpcap', required: false)
if not dep.found()
    dep = cc.find_library('pcap', required: true)
endif
global_deps += dep

global_deps += cc.find_library('dl', required: true)
global_deps += cc.find_library('m', required: true)
global_deps += cc.find_library('pthread', required: true)

add_project_arguments('-pthread', language: 'c')

subdir('tools')

subdir('lib')

subdir('app')

subdir('doc')

pkg = import('pkgconfig')
pkg.generate(name: meson.project_name(),
        filebase: 'lib' + meson.project_name().to_lower(),
        version: meson.project_version(),
        description: 'Pktgen a Data Plane Development Kit (DPDK) application.',
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
