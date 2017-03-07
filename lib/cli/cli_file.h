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

#ifndef _CLI_FILE_H_
#define _CLI_FILE_H_

/**
 * @file
 * RTE Command line interface
 *
 */

#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_FILE_SIZE   1024

enum {
	/* File operations opt */
    CLI_FILE_RD     = 0x0001,       /** Do a read on a file */
    CLI_FILE_WR     = 0x0002,       /** Do a write on a file */
    CLI_FILE_APPEND = 0x0004,       /** Append to a file */
    CLI_FILE_OPEN   = 0x0008,       /** Open a file */
    CLI_FILE_CLOSE  = 0x0010,       /** Close a file */
    CLI_FILE_CREATE = 0x0020,       /** Create a file */

	/* File seek operations */
	CLI_SEEK_SET    = 0x0100,       /** Set file pointer to a given offset */
    CLI_SEEK_CUR    = 0x0200,       /** Seek from current file pointer */
    CLI_SEEK_END    = 0x0400,       /** Seek from end of file */

	/* File information in cli_node.fflags */
	CLI_DATA_RDONLY = 0x1000,       /** file is read only */
    CLI_FREE_DATA   = 0x2000,       /** File data needs to be freed */
    CLI_DATA_EXPAND = 0x4000		/** File is expandable */
};

#define file_set(f, v)		do { (f) |= (v); } while((0))
#define file_clr(f, v)		do { (f) &= ~(v); } while((0))

static inline int
is_file_set(uint32_t opt, uint32_t cmpflags)
{
	return opt & cmpflags;
}

static inline int
is_file_rd(uint32_t opt)
{
	return is_file_set(opt, CLI_FILE_RD);
}

static inline int
is_file_wr(uint32_t opt)
{
	return is_file_set(opt, CLI_FILE_WR);
}

static inline int
is_file_append(uint32_t opt)
{
	return is_file_set(opt, CLI_FILE_APPEND);
}

static inline int
is_file_open(uint32_t opt)
{
	return is_file_set(opt, CLI_FILE_OPEN);
}

static inline int
is_file_close(uint32_t opt)
{
	return is_file_set(opt, CLI_FILE_CLOSE);
}

static inline int
is_file_create(uint32_t opt)
{
	return is_file_set(opt, CLI_FILE_CREATE);
}

static inline int
is_data_rdonly(uint32_t flags)
{
	return is_file_set(flags, CLI_DATA_RDONLY);
}

static inline int
is_file_eq(uint32_t opt, uint32_t cmpflags)
{
	return ((opt & cmpflags) == cmpflags);
}

static inline int
is_seek_set(uint32_t opt)
{
	return is_file_set(opt, CLI_SEEK_SET);
}

static inline int
is_seek_cur(uint32_t opt)
{
	return is_file_set(opt, CLI_SEEK_CUR);
}

static inline int
is_seek_end(uint32_t opt)
{
	return is_file_set(opt, CLI_SEEK_END);
}

/**
 * Open a file.
 *
 * @param path
 *   Path string for file
 * @param type
 *   Type of open string r, w, and/or + characters
 * @return
 *   Node pointer or NULL on error
 */
struct cli_node *cli_file_open(const char *path, const char *type);

/**
 * Close a file
 *
 * @param node
 *   Pointer to file node
 * @return
 *   0 on OK and -1 on error
 */
int cli_file_close(struct cli_node *node);

/**
 * read data from a file
 *
 * @param node
 *   Pointer to file node
 * @param buff
 *   Pointer to place to put the data
 * @param len
 *   Max Number of bytes to read
 * @return
 *   Number of bytes read and -1 on error
 */
int cli_file_read(struct cli_node *node, char * buff, int len);

/**
 * write data to a file
 *
 * @param node
 *   Pointer to file node
 * @param buff
 *   Pointer to place to get the data
 * @param len
 *   Max Number of bytes to write
 * @return
 *   Number of bytes written and -1 on error
 */
int cli_file_write(struct cli_node *node, char * buff, int len);

/**
 * write data to a file
 *
 * @param node
 *   Pointer to file node
 * @param offset
 *   Offset to move in file
 * @param whence
 *   Type of seek operation CLI_SEEK_SET, CLI_SEEK_CUR and CLI_SEEK_END
 * @return
 *   Offset in file after seek and -1 on error
 */
int cli_file_seek(struct cli_node *node, int offset, uint32_t whence);

/**
 * write data to a file
 *
 * @param node
 *   Pointer to file node
 * @param buff
 *   place to put the line data.
 * @param len
 *   Max buff size
 * @return
 *   Number of bytes read not including the newline
 */
int cli_readline(struct cli_node *node, char *buff, int len);

/**
 * create a data file in memory will be lost at reset.
 *
 * @param path
 *   Path string for file
 * @param type
 *   Type of open string r, w, and/or + characters
 * @return
 *   Node pointer or NULL on error
 */
struct cli_node *cli_file_create(const char *path, const char *type);

/**
 * Generic file function for basic file handling
 *
 * @param node
 *   Pointer to file node
 * @param buff
 *   place to put the line data.
 * @param len
 *   Max buff size
 * @param opt
 *   Flags for file handling
 * @return
 *   Number of bytes read not including the newline
 */
int cli_file_handler(struct cli_node *node,
                  char *buff, int len, uint32_t opt);

/**
 * Generic file function for basic file handling
 *
 * @param node
 *   Pointer to file node
 * @param buff
 *   place to put the line data.
 * @param len
 *   Max buff size
 * @param opt
 *   Flags for file handling
 * @return
 *   Number of bytes read not including the newline
 */
int cli_system(char *p);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_FILE_H_ */
