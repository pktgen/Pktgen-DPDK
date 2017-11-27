#   BSD LICENSE
#
#   Copyright(c) 2010-2012 Intel Corporation. All rights reserved.
#
#   SPDX-License-Identifier: BSD-3-Clause
#

lua_version := 5.3.4
tarball := lua-$(lua_version).tar.gz

all: get_tarball
	@make -C lua-$(lua_version)

get_tarball:
	@if [ ! -f $(tarball) ]; then \
		wget https://www.lua.org/ftp/lua-5.3.4.tar.gz; \
	fi
	@if [ ! -d lua-$(lua_version) ]; then \
		tar xf $(tarball); \
		(cd lua-5.3.4; patch -p1 < ../lua-5.3.4.patch); \
	fi

clean:
	@rm -fr lua-$(lua_version)
