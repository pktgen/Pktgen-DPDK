/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
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
