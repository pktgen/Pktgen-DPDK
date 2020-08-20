project('pktgen', 'C',
    version: run_command(find_program('cat', 'more'),
        files('VERSION')).stdout().strip(),

	license: 'BSD',
	default_options: ['buildtype=release', 'default_library=static'],
	meson_version: '>= 0.47.1'
)

pktgen_conf = configuration_data()

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')

pktgen_shared_libraries = []
pktgen_static_libraries = []

pver = meson.project_version().split('.')
major_version = '@0@.@1@'.format(pver.get(0), pver.get(1))

add_project_arguments('-mavx', language: 'c')
add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')
if cc.has_argument('-Wno-format-truncation')
    add_project_arguments('-Wno-format-truncation', language: 'c')
endif
if get_option('enable_lua')
    add_project_arguments('-DLUA_ENABLED', language: 'c')
endif


bsd_dep = dependency('libbsd', required: true)

dpdk_dep = dependency('libdpdk', required: true)

subdir('tools')

subdir('lib')

subdir('app')

subdir('doc')

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
