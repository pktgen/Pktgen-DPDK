/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2021-2023> Intel Corporation
 */

#ifndef __FGEN_H
#define __FGEN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <rte_mbuf.h>

#define _INFO(...)           \
    do {                     \
        printf("Info: ");    \
        printf(__VA_ARGS__); \
    } while ((0))

#define _WARN(...)           \
    do {                     \
        printf("Warning: "); \
        printf(__VA_ARGS__); \
    } while ((0))

#define _ERR_RET(...)        \
    do {                     \
        printf("Error: ");   \
        printf(__VA_ARGS__); \
        return -1;           \
    } while ((0))

#define _RET(...)            \
    do {                     \
        printf("Error: ");   \
        printf(__VA_ARGS__); \
        return;              \
    } while ((0))

#define _ERR_GOTO(lbl, ...)  \
    do {                     \
        printf("Error: ");   \
        printf(__VA_ARGS__); \
        goto lbl;            \
    } while ((0))

enum {
    FGEN_MAX_STRING_LENGTH = 1024, /**< Maximum string length for text string */
    FGEN_MAX_LAYERS        = 32,   /**< Maximum number of layers in the text string */
    FGEN_MAX_PARAMS        = 16,   /**< Maximum number of parameters in the text string */
    FGEN_MAX_KVP_TOKENS    = 4,    /**< Maximum number of tokens in a key/value pair + 1 */
    FGEN_FILLER_PATTERN    = 0x5a, /**< Filler pattern byte value */
    FGEN_FRAME_NAME_LENGTH = 32,   /**< Frame name length */
    FGEN_EXTRA_SPACE       = 64,   /**< Extra space for building the fgen string */
    FGEN_MAX_BUF_LEN       = 4096, /**< Maximum number of bytes in the fgen string */
};

typedef enum {
    FGEN_ETHER_TYPE,     /**< Ethernet type layer */
    FGEN_DOT1Q_TYPE,     /**< Dot1Q type layer */
    FGEN_DOT1AD_TYPE,    /**< Dot1AD type layer */
    FGEN_IPV4_TYPE,      /**< IPv4 type layer */
    FGEN_IPV6_TYPE,      /**< IPv6 type layer */
    FGEN_UDP_TYPE,       /**< UDP type layer */
    FGEN_TCP_TYPE,       /**< TCP type layer */
    FGEN_VXLAN_TYPE,     /**< VxLan type layer */
    FGEN_ECHO_TYPE,      /**< ECHO type layer */
    FGEN_TSC_TYPE,       /**< Timestamp type layer */
    FGEN_RAW_TYPE,       /**< Raw type layer */
    FGEN_PAYLOAD_TYPE,   /**< Payload type layer */
    FGEN_TYPE_COUNT,     /**< Number of layers total */
    FGEN_ERROR_TYPE = -1 /**< Error type */
} opt_type_t;

/**< The set of strings matching the opt_type_t structure */
#define FGEN_TYPE_STRINGS                                                                        \
    {                                                                                            \
        "Ether", "Dot1q", "Dot1ad", "IPv4", "IPv6", "UDP", "TCP", "Vxlan", "Echo", "TSC", "Raw", \
            "Payload", "Done"                                                                    \
    }

#define FGEN_DONE_TYPE FGEN_TYPE_COUNT /**< A parsing done flag */

/* Forward declarations */
struct fgen_s;
struct fopt_s;
struct frame_s;

typedef int (*fgen_fn_t)(struct fgen_s *fg, struct frame_s *f, int lidx);

typedef struct ftable_s {
    opt_type_t typ;  /**< Type of layer */
    const char *str; /**< Name of the layer and string for comparing */
    fgen_fn_t fn;    /**< Routine to call for a given layer */
} ftable_t;

typedef struct fopt_s {
    opt_type_t typ;  /**< Type of layer */
    uint16_t offset; /**< Offset into the mbuf for this layer */
    uint16_t length; /**< Length of the layer */
    char *param_str; /**< Parameter string for the layer */
    ftable_t *tbl;   /**< The table containing the layer parsing routine */
} fopt_t;

typedef struct proto_s {
    uint16_t offset; /**< Offset to the protocol header in buffer */
    uint16_t length; /**< Length of the protocol header in buffer */
} proto_t;

typedef struct frame_s {
    char name[FGEN_FRAME_NAME_LENGTH]; /**< Name of the frame */
    char *frame_text;                  /**< Frame text string, this string allocated by strdup() */
    void *data;                        /**< Frame pointer */
    uint16_t fidx;                     /**< Frame index value */
    uint16_t bufsz;                    /**< Total length of the frame buffer */
    uint16_t data_len;                 /**< Total length of frame */
    uint16_t tsc_off;                  /**< Offset to the Timestamp */
    proto_t l2;                        /**< Information about L2 header */
    proto_t l3;                        /**< Information about L3 header */
    proto_t l4;                        /**< Information about L4 header */
} frame_t;

typedef struct fgen_s {
    int flags; /**< Flags for debugging and parsing the text */
    void *mm;
    frame_t *frames;      /**< The frame information for each frame built */
    uint16_t max_frames;  /**< Maximum number of frames */
    uint16_t frame_bufsz; /**< Allocated size of each frame buffer */
    uint16_t nb_frames;   /**< Number of frames added */
    uint16_t num_layers;  /**< Number of layers */

    fopt_t opts[FGEN_MAX_LAYERS];  /**< The option information one for each layer */
    char *layers[FGEN_MAX_LAYERS]; /**< information about each layer */
    char *params[FGEN_MAX_PARAMS]; /**< Parameters for each layer */
} fgen_t;

enum {
    FGEN_VERBOSE   = (1 << 0), /**< Debug flag to enable verbose output */
    FGEN_DUMP_DATA = (1 << 1), /**< Debug flag to hexdump the data */
};

#define fgen_frame_count(fg) (fg)->nb_frames

/**
 * Return the data pointer to the packet data.
 *
 * @param f
 *   The frame_t structure pointer.
 */
#define fgen_data(f) (f)->data

/**
 * Return the packet data length.
 *
 * @param f
 *   The frame_t structure pointer.
 */
#define fgen_data_len(f) (f)->data_len

/**
 * Return the packet data pointer at the given offset.
 *
 * @param f
 *   The frame_t structure pointer.
 * @param t
 *   The pointer type cast.
 * @param o
 *   The offset into the packet.
 */
#define fgen_mtod_offset(f, t, o) ((t)((char *)fgen_data(f) + (o)))

/**
 * Return the first byte address of the packet data.
 *
 * @param f
 *   The frame_t structure pointer.
 * @param t
 *   The pointer type cast.
 */
#define fgen_mtod(f, t) fgen_mtod_offset(f, t, 0)

/**
 * Create a frame generator object with number of frames and size of each frame.
 *
 * @param max_frames
 *   The maximum number of frames allowed to be added to the fgen_t structure.
 * @param frame_sz
 *   The max size of the frame buffer.
 * @param flags
 *   Flags used for debugging the frame generator object.
 *   FGEN_VERBOSE, FGEN_DUMP_DATA, ...
 * @return
 *   NULL on error or Pointer to fgen_t structure on success.
 */
fgen_t *fgen_create(uint16_t max_frames, uint16_t frame_sz, int flags);

/**
 * Destroy the frame generator object.
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 */
void fgen_destroy(fgen_t *fg);

/**
 * Load a fgen text file and grab the fgen frame strings/names.
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 * @param filename
 *   The name of the file to load, must be a fgen text file
 * @return
 *   -1 on error or number of frames loaded
 */
int fgen_load_file(fgen_t *fg, const char *filename);

/**
 * Load a fgen string array and create a fgen_file_t structure pointer.
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 * @param frames
 *   The array of fgen text strings
 * @param len
 *   The number of entries in the array. The len can be zero, but the array
 *   must be NULL terminated.
 * @return
 *   -1 on error or number of frames loaded
 */
int fgen_load_strings(fgen_t *fg, const char **frames, int len);

/**
 * Parse the fgen string to generate the frame and add to the list.
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 * @param name
 *   The name of the frame text to create.
 * @param text
 *   The text string to parse to generate the frame data.
 * @return
 *   0 on success or -1 on error.
 */
int fgen_add_frame(fgen_t *fg, const char *name, const char *text);

/**
 * Copy the frame generator object into the list of mbufs and adjust the mbufs accordingly.
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 * @param low
 *   The starting index of the packet data to copy to the mbuf array.
 * @param high
 *   The ending index of the packet data to copy to the mbuf array.
 * @param mbufs
 *   The mbuf array to copy data and the caller must supply the mbufs array pointers.
 * @param nb_pkts
 *   The number of mbuf pointers in the mbuf array
 * @return
 *   0 on success or -1 on error.
 */
int fgen_alloc(fgen_t *fg, int low, int high, struct rte_mbuf **mbufs, uint32_t nb_pkts);

/**
 * Free the mbuf array, plus update stats like latency, ...
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 * @param mbufs
 *   The mbuf array to parse and free mbufs.
 * @param nb_pkts
 *   The number of mbuf pointers in the mbuf array
 * @return
 *   0 on success and free the mbuf array or -1 on error.
 */
int fgen_free(fgen_t *fg, struct rte_mbuf **mbufs, uint32_t nb_pkts);

/**
 * Print out a fgen frame text string
 *
 * @param msg
 *   A header message, can be NULL.
 * @param fpkt
 *   The frame_t pointer to print.
 */
void fgen_print_frame(const char *msg, frame_t *f);

/**
 * Return the frame pointer for the given index value
 *
 * @param fg
 *   The fgen_t pointer returned from fgen_create()
 * @return
 *   frame_t pointer on success or NULL on error.
 */
static inline frame_t *
fgen_get_frame(fgen_t *fg, uint16_t idx)
{
    return (!fg || (idx >= fg->nb_frames)) ? NULL : &fg->frames[idx];
}

#ifdef __cplusplus
}
#endif

#endif /* __FGEN_H */
