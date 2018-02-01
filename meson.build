project('pktgen', 'C',
	license: 'BSD',
	default_options: ['buildtype=release', 'default_library=static'],
	meson_version: '>= 0.41'
)

add_project_arguments('-D_GNU_SOURCE', '-O3', language: 'c')
cc = meson.get_compiler('c')

dpdk_dep = dependency('libdpdk', version: '>=17.11', required: false)
if not dpdk_dep.found()
	dpdk_sp = subproject('dpdk')
	dpdk_dep = declare_dependency(
			compile_args: ['-include', 'rte_config.h'] +
				dpdk_sp.get_variable('machine_args'),
			dependencies: [dpdk_sp.get_variable('static_rte_ethdev'),
				dpdk_sp.get_variable('static_rte_timer'),
				dpdk_sp.get_variable('static_rte_pmd_i40e'),
				dpdk_sp.get_variable('static_rte_pmd_pcap'),
				dpdk_sp.get_variable('static_rte_cmdline'),
				dpdk_sp.get_variable('static_rte_lpm')],
			link_args: dpdk_sp.get_variable('dpdk_extra_ldflags'))
endif

subdir('lib')
subdir('app')
