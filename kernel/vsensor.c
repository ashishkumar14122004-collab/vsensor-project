// SPDX-License-Identifier: GPL-2.0
/**
 * @file vsensor.c
 * @brief VSensor – Virtual Sensor Linux Character Device Driver
 *
 * This kernel module registers a character device at /dev/vsensor.
 * On each read(), it returns a JSON string containing simulated
 * temperature (20–85 °C) and CPU load (0–100 %) values, generated
 * using a lightweight pseudo-random number generator seeded by
 * ktime_get().
 *
 * A ring buffer of the last VSENSOR_RING_SIZE readings is maintained
 * in kernel space, accessible via ioctl.
 *
 * Supported ioctl commands:
 *   VSENSOR_RESET     – clear ring buffer and statistics
 *   VSENSOR_GET_STATS – copy min/max/avg stats to userspace
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ktime.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "vsensor.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VSensor Project");
MODULE_DESCRIPTION("Virtual Sensor Character Device Driver");
MODULE_VERSION("1.0");

/* ------------------------------------------------------------------ */
/*  Internal data structures                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief A single sensor reading stored in the kernel ring buffer.
 */
struct vsensor_reading {
    s64          timestamp_ns;    /**< ktime_get() nanoseconds       */
    int          temperature_x10; /**< Temperature × 10 (e.g. 253)   */
    int          cpu_load_x10;    /**< CPU load × 10     (e.g. 672)  */
};

/**
 * @brief Device-wide state (one instance).
 */
struct vsensor_dev {
    struct cdev              cdev;
    struct class            *dev_class;
    dev_t                    devno;
    struct mutex             lock;
    struct vsensor_reading   ring[VSENSOR_RING_SIZE];
    unsigned int             ring_head;   /**< Next write position     */
    unsigned int             ring_count;  /**< Number of valid entries */
};

static struct vsensor_dev g_dev;

/* ------------------------------------------------------------------ */
/*  Pseudo-random sensor simulation                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Generate a pseudo-random integer in [lo, hi] (inclusive).
 *
 * Uses get_random_u32() (crypto-quality RNG from the kernel entropy pool).
 */
static int vsensor_rand_range(int lo, int hi)
{
    unsigned int r;
    get_random_bytes(&r, sizeof(r));
    return lo + (int)(r % (unsigned int)(hi - lo + 1));
}

/**
 * @brief Generate a new sensor reading and push it into the ring buffer.
 * @note Caller must hold g_dev.lock.
 */
static void vsensor_generate_reading(void)
{
    struct vsensor_reading *slot;

    slot = &g_dev.ring[g_dev.ring_head];
    slot->timestamp_ns    = ktime_get_real_ns();
    slot->temperature_x10 = vsensor_rand_range(200, 850); /* 20.0 – 85.0 */
    slot->cpu_load_x10    = vsensor_rand_range(0,   1000); /* 0.0  – 100.0 */

    g_dev.ring_head = (g_dev.ring_head + 1) % VSENSOR_RING_SIZE;
    if (g_dev.ring_count < VSENSOR_RING_SIZE)
        g_dev.ring_count++;
}

/* ------------------------------------------------------------------ */
/*  File operations                                                    */
/* ------------------------------------------------------------------ */

static int vsensor_open(struct inode *inode, struct file *filp)
{
    pr_info("[vsensor] device opened (pid=%d)\n", current->pid);
    filp->private_data = &g_dev;
    return 0;
}

static int vsensor_release(struct inode *inode, struct file *filp)
{
    pr_info("[vsensor] device released (pid=%d)\n", current->pid);
    return 0;
}

/**
 * @brief Read one sensor reading from the device.
 *
 * Generates a new reading, formats it as JSON, and copies it to the
 * userspace buffer. Each call returns a complete JSON line.
 *
 * Example output:
 *   {"temp":37.4,"load":62.1,"ts":1728043200000}\n
 */
static ssize_t vsensor_read(struct file *filp, char __user *buf,
                             size_t count, loff_t *ppos)
{
    struct vsensor_dev     *dev = filp->private_data;
    struct vsensor_reading *latest;
    char                    kbuf[VSENSOR_BUF_SIZE];
    int                     len;
    unsigned long           ts_ms;

    if (*ppos > 0)          /* Only one read per open by default */
        return 0;

    mutex_lock(&dev->lock);
    vsensor_generate_reading();
    latest = &dev->ring[(dev->ring_head + VSENSOR_RING_SIZE - 1)
                        % VSENSOR_RING_SIZE];
    ts_ms = (unsigned long)(latest->timestamp_ns / 1000000ULL);
    mutex_unlock(&dev->lock);

    len = snprintf(kbuf, sizeof(kbuf),
                   "{\"temp\":%d.%d,\"load\":%d.%d,\"ts\":%lu}\n",
                   latest->temperature_x10 / 10,
                   abs(latest->temperature_x10 % 10),
                   latest->cpu_load_x10 / 10,
                   abs(latest->cpu_load_x10 % 10),
                   ts_ms);

    if (len >= (int)count)
        return -EINVAL;

    if (copy_to_user(buf, kbuf, len + 1))
        return -EFAULT;

    *ppos += len;
    return len;
}

/**
 * @brief Write a threshold value to the device (reserved for future use).
 */
static ssize_t vsensor_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *ppos)
{
    pr_info("[vsensor] write() called (reserved, count=%zu)\n", count);
    return count; /* Acknowledge, but ignore for now */
}

/**
 * @brief ioctl handler.
 *
 * VSENSOR_RESET:     Clear ring buffer.
 * VSENSOR_GET_STATS: Return min/max/avg statistics to userspace.
 */
static long vsensor_ioctl(struct file *filp, unsigned int cmd,
                           unsigned long arg)
{
    struct vsensor_dev   *dev = filp->private_data;
    struct vsensor_stats  stats;
    unsigned int          i, n;
    long                  temp_sum = 0, load_sum = 0;

    switch (cmd) {

    case VSENSOR_RESET:
        mutex_lock(&dev->lock);
        memset(dev->ring, 0, sizeof(dev->ring));
        dev->ring_head  = 0;
        dev->ring_count = 0;
        mutex_unlock(&dev->lock);
        pr_info("[vsensor] ring buffer reset\n");
        return 0;

    case VSENSOR_GET_STATS:
        mutex_lock(&dev->lock);
        n = dev->ring_count;
        if (n == 0) {
            mutex_unlock(&dev->lock);
            memset(&stats, 0, sizeof(stats));
            if (copy_to_user((void __user *)arg, &stats, sizeof(stats)))
                return -EFAULT;
            return 0;
        }

        stats.temp_min_x10 = dev->ring[0].temperature_x10;
        stats.temp_max_x10 = dev->ring[0].temperature_x10;
        stats.load_min_x10 = dev->ring[0].cpu_load_x10;
        stats.load_max_x10 = dev->ring[0].cpu_load_x10;

        for (i = 0; i < n; i++) {
            int t = dev->ring[i].temperature_x10;
            int l = dev->ring[i].cpu_load_x10;
            if (t < stats.temp_min_x10) stats.temp_min_x10 = t;
            if (t > stats.temp_max_x10) stats.temp_max_x10 = t;
            if (l < stats.load_min_x10) stats.load_min_x10 = l;
            if (l > stats.load_max_x10) stats.load_max_x10 = l;
            temp_sum += t;
            load_sum += l;
        }
        stats.temp_avg_x10  = (int)(temp_sum / n);
        stats.load_avg_x10  = (int)(load_sum / n);
        stats.sample_count  = n;
        mutex_unlock(&dev->lock);

        if (copy_to_user((void __user *)arg, &stats, sizeof(stats)))
            return -EFAULT;
        return 0;

    default:
        return -ENOTTY;
    }
}

/* ------------------------------------------------------------------ */
/*  File operations table                                              */
/* ------------------------------------------------------------------ */

static const struct file_operations vsensor_fops = {
    .owner          = THIS_MODULE,
    .open           = vsensor_open,
    .release        = vsensor_release,
    .read           = vsensor_read,
    .write          = vsensor_write,
    .unlocked_ioctl = vsensor_ioctl,
};

/* ------------------------------------------------------------------ */
/*  Module init / exit                                                 */
/* ------------------------------------------------------------------ */

static int __init vsensor_init(void)
{
    int ret;
    struct device *device;

    pr_info("[vsensor] loading module v1.0\n");

    /* Allocate a dynamic major number */
    ret = alloc_chrdev_region(&g_dev.devno, 0, 1, VSENSOR_DEVICE_NAME);
    if (ret < 0) {
        pr_err("[vsensor] alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }

    /* Initialize and add the cdev */
    cdev_init(&g_dev.cdev, &vsensor_fops);
    g_dev.cdev.owner = THIS_MODULE;
    ret = cdev_add(&g_dev.cdev, g_dev.devno, 1);
    if (ret < 0) {
        pr_err("[vsensor] cdev_add failed: %d\n", ret);
        goto err_unreg;
    }

    /* Create device class (shows up in /sys/class/) */
    g_dev.dev_class = class_create(THIS_MODULE, VSENSOR_CLASS_NAME);
    if (IS_ERR(g_dev.dev_class)) {
        ret = PTR_ERR(g_dev.dev_class);
        pr_err("[vsensor] class_create failed: %d\n", ret);
        goto err_cdev;
    }

    /* Create device node /dev/vsensor via udev */
    device = device_create(g_dev.dev_class, NULL, g_dev.devno,
                           NULL, VSENSOR_DEVICE_NAME);
    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        pr_err("[vsensor] device_create failed: %d\n", ret);
        goto err_class;
    }

    mutex_init(&g_dev.lock);
    memset(g_dev.ring, 0, sizeof(g_dev.ring));
    g_dev.ring_head  = 0;
    g_dev.ring_count = 0;

    pr_info("[vsensor] registered at major=%d minor=%d → /dev/%s\n",
            MAJOR(g_dev.devno), MINOR(g_dev.devno), VSENSOR_DEVICE_NAME);
    return 0;

err_class:
    class_destroy(g_dev.dev_class);
err_cdev:
    cdev_del(&g_dev.cdev);
err_unreg:
    unregister_chrdev_region(g_dev.devno, 1);
    return ret;
}

static void __exit vsensor_exit(void)
{
    device_destroy(g_dev.dev_class, g_dev.devno);
    class_destroy(g_dev.dev_class);
    cdev_del(&g_dev.cdev);
    unregister_chrdev_region(g_dev.devno, 1);
    mutex_destroy(&g_dev.lock);
    pr_info("[vsensor] module unloaded\n");
}

module_init(vsensor_init);
module_exit(vsensor_exit);
