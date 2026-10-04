/* SPDX-License-Identifier: GPL-2.0 */
/**
 * @file vsensor.h
 * @brief VSensor kernel module – shared definitions and ioctl interface.
 *
 * This header is shared between the kernel module and the userspace
 * application. It defines the ioctl command numbers and data structures
 * exchanged across the kernel/userspace boundary.
 */

#ifndef VSENSOR_H
#define VSENSOR_H

#include <linux/ioctl.h>

/** Device name as it appears under /dev */
#define VSENSOR_DEVICE_NAME  "vsensor"

/** Class name for udev */
#define VSENSOR_CLASS_NAME   "vsensor_class"

/** Maximum size of a single JSON read response (bytes) */
#define VSENSOR_BUF_SIZE     128

/** Number of readings stored in the kernel ring buffer */
#define VSENSOR_RING_SIZE    64

/* ------------------------------------------------------------------ */
/*  Data structures exchanged via ioctl                                */
/* ------------------------------------------------------------------ */

/**
 * @brief Statistics returned by VSENSOR_GET_STATS ioctl.
 *
 * All temperature and load values are stored as integer * 10 to avoid
 * floating-point in kernel space. Divide by 10.0 in userspace.
 */
struct vsensor_stats {
    int  temp_min_x10;   /**< Minimum temperature × 10  */
    int  temp_max_x10;   /**< Maximum temperature × 10  */
    int  temp_avg_x10;   /**< Average temperature × 10  */
    int  load_min_x10;   /**< Minimum CPU load × 10     */
    int  load_max_x10;   /**< Maximum CPU load × 10     */
    int  load_avg_x10;   /**< Average CPU load × 10     */
    unsigned int sample_count; /**< Total samples in buffer */
};

/* ------------------------------------------------------------------ */
/*  ioctl command definitions                                          */
/* ------------------------------------------------------------------ */

/** Magic number for VSensor ioctl commands */
#define VSENSOR_MAGIC  'V'

/** Reset the ring buffer and clear all statistics */
#define VSENSOR_RESET      _IO(VSENSOR_MAGIC,  0)

/** Retrieve statistics from the ring buffer */
#define VSENSOR_GET_STATS  _IOR(VSENSOR_MAGIC, 1, struct vsensor_stats)

#endif /* VSENSOR_H */
