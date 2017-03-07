/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cli.h"
#include "cli_file.h"

struct cli_node *
cli_file_open(const char *path, const char *type)
{
    struct cli_node *node;
    uint32_t flags = 0;

	if (!path)
		return NULL;

	if (!cli_find_node(path, &node))
		return NULL;

	if (!is_file(node))
		return NULL;

	if (type && strlen(type)) {
		if (strchr(type, 'r'))
			file_set(flags, CLI_FILE_RD);
		if (strchr(type, 'w'))
			file_set(flags, CLI_FILE_WR);
		if (strchr(type, '+'))
			file_set(flags, CLI_FILE_APPEND);
	} else
		file_set(flags, CLI_FILE_RD);

	file_set(flags, CLI_FILE_OPEN);

	if (node->ffunc(node, NULL, 0, flags))
		return NULL;

	return node;
}

int
cli_file_close(struct cli_node *node)
{
	uint32_t flags = CLI_FILE_CLOSE;

	if (!node)
		return -1;
	return node->ffunc(node, NULL, 0, flags);
}

int
cli_file_read(struct cli_node *node, char *buff, int len)
{
	uint32_t flags = CLI_FILE_RD;

	if (!node || !is_file(node))
		return -1;
	return node->ffunc(node, buff, len, flags);
}

int
cli_file_write(struct cli_node *node, char *buff, int len)
{
	uint32_t flags = CLI_FILE_WR;

	if (!node || !is_file(node))
		return -1;
	if (is_data_rdonly(node->fflags))
		return -1;
	return node->ffunc(node, buff, len, flags);
}

int
cli_file_seek(struct cli_node *node, int offset, uint32_t whence)
{
	if (!node || !is_file(node))
		return -1;

	switch (whence) {
	case CLI_SEEK_SET:
	case CLI_SEEK_CUR:
	case CLI_SEEK_END:
		break;
	default:
		return -1;
	}
	return node->ffunc(node, NULL, offset, whence);
}

int
cli_readline(struct cli_node *node, char *buff, int len)
{
	int i, n;
	char c;

	if (!node || !buff || !is_file(node))
		return -1;
	/* Needs to be optimized for performance ??? */
	for (i = 0, c = '\0'; i < len && c != '\n'; i++) {
		n = cli_file_read(node, &c, 1);
		if (n <= 0)
			break;
		buff[i] = c;
	}
	buff[i] = '\0';
	return i;
}

/* Add generic function for handling files */
int
cli_file_handler(struct cli_node *node, char *buff, int len, uint32_t opt)
{
	char *p;

	if (!node || !is_file(node))
		return -1;

	if (opt & (CLI_SEEK_SET | CLI_SEEK_CUR | CLI_SEEK_END)) {
		size_t saved = node->foffset;

		if (is_seek_set(opt)) {
			if (len < 0)
				return -1;
			node->foffset = len;
		} else if (is_seek_cur(opt)) {
			if (len < 0) {
				len = abs(len);
				if ((size_t)len > node->file_size)
					node->foffset = 0;
				else
					node->foffset -= len;
			} else
				node->foffset += len;
		} else if (is_seek_end(opt)) {
			if (len < 0) {
				len = abs(len);
				if ((size_t)len > node->file_size)
					node->foffset = 0;
				else
					node->foffset = node->file_size - len;
			} else
				node->foffset = node->file_size + len;
		}

		if (node->foffset > node->file_size) {
			if (!(node->fflags & CLI_FILE_APPEND)) {
				node->foffset = saved;
				if (node->fflags & (CLI_FREE_DATA | CLI_DATA_EXPAND)) {
					char *data;
					data = realloc(node->file_data, node->foffset);
					if (!data)
						return -1;
					node->file_data = data;
					node->file_size = node->foffset;
				} else
					return -1;	/* TODO: add code to expand the file */
			} else {
				node->foffset = saved;
				return -1;
			}
		}
	} else if (is_seek_cur(opt))
		node->foffset += len;
	else if (is_seek_end(opt))
		node->foffset += len;
	else if (is_file_close(opt)) {
		if (node->file_data && (node->fflags & CLI_FREE_DATA)) {
			free(node->file_data);
			node->file_data = NULL;
			node->file_size = 0;
		}
		node->foffset = 0;
	} else if (is_file_open(opt)) {
		if (is_file_append(opt)) {
			node->fflags |= CLI_FILE_APPEND;
			node->foffset = node->file_size;
		}
		if (is_file_wr(opt))
			node->fflags |= CLI_FILE_WR;
	} else if (is_file_rd(opt)) {
		if (len <= 0)
			return 0;

		len = RTE_MIN(len, (int)(node->file_size - node->foffset));

		p = node->file_data + node->foffset;

		memcpy(buff, p, len);

		node->foffset += len;
	} else if (is_file_wr(opt)) {
		if (!is_data_rdonly(node->fflags))
			return -1;
		if (len <= 0)
			return 0;
		if ((node->foffset + len) < node->file_size) {
			p = node->file_data + node->foffset;
			memcpy(p, buff, len);
			node->foffset += len;
		} else {
			p = realloc(node->file_data, (node->foffset + len));
			node->file_data = p;
			node->file_size = node->foffset + len;
			node->foffset += len;
		}
	}

	return len;
}

struct cli_node *
cli_file_create(const char *path, const char *type){
	struct cli_node *node, *parent;
	char *file, *mypath;
	char *data = NULL;

	node = cli_file_open(path, type);
	if (node)
		return node;

	mypath = alloca(strlen(path) + 1);

	strcpy(mypath, path);

	file = basename(mypath);

	data = malloc(CLI_FILE_SIZE);
	if (data) {
		parent = cli_last_node_in_path(path);
		if (parent) {
			node = cli_add_file(file, parent, cli_file_handler, "");
			if (node) {
				node->file_data = data;
				node->file_size = CLI_FILE_SIZE;
				node->fflags = CLI_FREE_DATA;
				if (strchr(type, 'r') && !strchr(type, 'w'))
					node->fflags |= CLI_DATA_RDONLY;
				node->foffset = 0;
				node->fflags = 0;
				node->fstate = 0;
				return node;
			}
		}
	}

	free(data);
	return NULL;
}

int
cli_system(char *p)
{
	char buf[256];
	size_t n, tot = 0;
	FILE *f;

	f = popen(p, "r");
	if (!f)
		return -1;

	while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
		cli_write(buf, n);
		tot += n;
	}

	pclose(f);

	return n;
}
