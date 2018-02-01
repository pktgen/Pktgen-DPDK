dirs = ['common', 'cli']

foreach d: dirs
	sources = []
	deps = []
	cflags = []
	subdir(d)

	inc = include_directories(d)
	lib = static_library(d, sources,
		include_directories: inc,
		c_args: cflags,
		dependencies: deps)
	set_variable('dep_' + d,
			declare_dependency(include_directories: inc,
				link_with: lib,
				dependencies: deps))
endforeach
