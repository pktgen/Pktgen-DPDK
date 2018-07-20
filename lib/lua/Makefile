#   BSD LICENSE
#
#   Copyright(c) 2010-2012 Intel Corporation. All rights reserved.
#
#   SPDX-License-Identifier: BSD-3-Clause
#

lua_version := 5.3.5
tarball := lua-$(lua_version).tar.gz

all: get_tarball
	@make -C lua-$(lua_version)

get_tarball:
	@if [ ! -f $(tarball) ]; then \
		wget https://www.lua.org/ftp/$(tarball); \
	fi
	@if [ ! -d lua-$(lua_version) ]; then \
		tar xf $(tarball); \
		(cd lua-$(lua_version); patch -p1 < ../lua-$(lua_version).patch); \
	fi

clean:
	@rm -fr lua-$(lua_version)
