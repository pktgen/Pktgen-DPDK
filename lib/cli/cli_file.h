/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2025>, Intel Corporation.
 */

#ifndef _CLI_FILE_H_
#define _CLI_FILE_H_

/**
 * @file
 * CLI file node helpers.
 *
 * Implements in-memory file nodes used by the CLI tree. File nodes can be
 * backed by a callback for dynamic content or can store data directly.
 */

#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_FILE_SIZE 1024

enum {
    /* File operations opt */
    CLI_FILE_RD     = 0x0001, /**< Perform a read on the file */
    CLI_FILE_WR     = 0x0002, /**< Perform a write on the file */
    CLI_FILE_APPEND = 0x0004, /**< Append data to the file */
    CLI_FILE_OPEN   = 0x0008, /**< Open the file */
    CLI_FILE_CLOSE  = 0x0010, /**< Close the file */
    CLI_FILE_CREATE = 0x0020, /**< Create a new file */

    /* File seek operations */
    CLI_SEEK_SET = 0x0100, /**< Seek to an absolute offset */
    CLI_SEEK_CUR = 0x0200, /**< Seek relative to the current position */
    CLI_SEEK_END = 0x0400, /**< Seek relative to the end of the file */

    /* File information in cli_node.fflags */
    CLI_DATA_RDONLY = 0x1000, /**< File data is read-only */
    CLI_FREE_DATA   = 0x2000, /**< File data buffer must be freed on close */
    CLI_DATA_EXPAND = 0x4000  /**< File buffer can be expanded dynamically */
};

#define file_set(f, v) \
    do {               \
        (f) |= (v);    \
    } while ((0))
#define file_clr(f, v) \
    do {               \
        (f) &= ~(v);   \
    } while ((0))

/** Test whether any flag in @p cmpflags is set in @p opt. Non-zero if set. */
static inline int
is_file_set(uint32_t opt, uint32_t cmpflags)
{
    return opt & cmpflags;
}

/** Return non-zero if CLI_FILE_RD is set in @p opt. */
static inline int
is_file_rd(uint32_t opt)
{
    return is_file_set(opt, CLI_FILE_RD);
}

/** Return non-zero if CLI_FILE_WR is set in @p opt. */
static inline int
is_file_wr(uint32_t opt)
{
    return is_file_set(opt, CLI_FILE_WR);
}

/** Return non-zero if CLI_FILE_APPEND is set in @p opt. */
static inline int
is_file_append(uint32_t opt)
{
    return is_file_set(opt, CLI_FILE_APPEND);
}

/** Return non-zero if CLI_FILE_OPEN is set in @p opt. */
static inline int
is_file_open(uint32_t opt)
{
    return is_file_set(opt, CLI_FILE_OPEN);
}

/** Return non-zero if CLI_FILE_CLOSE is set in @p opt. */
static inline int
is_file_close(uint32_t opt)
{
    return is_file_set(opt, CLI_FILE_CLOSE);
}

/** Return non-zero if CLI_FILE_CREATE is set in @p opt. */
static inline int
is_file_create(uint32_t opt)
{
    return is_file_set(opt, CLI_FILE_CREATE);
}

/** Return non-zero if CLI_DATA_RDONLY is set in @p flags. */
static inline int
is_data_rdonly(uint32_t flags)
{
    return is_file_set(flags, CLI_DATA_RDONLY);
}

/** Return non-zero if all bits in @p cmpflags are set in @p opt. */
static inline int
is_file_eq(uint32_t opt, uint32_t cmpflags)
{
    return ((opt & cmpflags) == cmpflags);
}

/** Return non-zero if CLI_SEEK_SET is set in @p opt. */
static inline int
is_seek_set(uint32_t opt)
{
    return is_file_set(opt, CLI_SEEK_SET);
}

/** Return non-zero if CLI_SEEK_CUR is set in @p opt. */
static inline int
is_seek_cur(uint32_t opt)
{
    return is_file_set(opt, CLI_SEEK_CUR);
}

/** Return non-zero if CLI_SEEK_END is set in @p opt. */
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
int cli_file_read(struct cli_node *node, char *buff, int len);

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
int cli_file_write(struct cli_node *node, char *buff, int len);

/**
 * Seek within a CLI file node.
 *
 * @param node
 *   Pointer to file node
 * @param offset
 *   Byte offset to apply (interpretation depends on @p whence)
 * @param whence
 *   Seek mode: CLI_SEEK_SET, CLI_SEEK_CUR, or CLI_SEEK_END
 * @return
 *   Resulting file offset after the seek, or -1 on error
 */
int cli_file_seek(struct cli_node *node, int offset, uint32_t whence);

/**
 * Read one line from a CLI file node into a buffer.
 *
 * Reads characters until a newline or end-of-file. The newline is consumed
 * but not stored.
 *
 * @param node
 *   Pointer to the file node to read from
 * @param buff
 *   Buffer to store the line data
 * @param len
 *   Maximum number of bytes the buffer can hold
 * @return
 *   Number of bytes stored in @p buff (excluding the newline), or -1 on error
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
int cli_file_handler(struct cli_node *node, char *buff, int len, uint32_t opt);

/**
 * Execute a host shell command string.
 *
 * Passes @p p to the system() call. Used by CLI built-in commands that
 * need to run external programs.
 *
 * @param p
 *   Null-terminated shell command string to execute.
 * @return
 *   Return value from system(), or -1 on error.
 */
int cli_system(char *p);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_FILE_H_ */
