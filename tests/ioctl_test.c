/**
 * @file ioctl_test.c
 * @brief Integration test for vsensor ioctl commands.
 *
 * Usage:
 *   ./ioctl_test reset   – sends VSENSOR_RESET, clears ring buffer
 *   ./ioctl_test stats   – sends VSENSOR_GET_STATS, prints statistics
 *
 * Build:
 *   gcc -o ioctl_test ioctl_test.c -I../kernel
 *
 * Run (driver must be loaded first):
 *   sudo ./ioctl_test stats
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

/* Include shared definitions from the kernel module header */
#include "../kernel/vsensor.h"

#define DEVICE_PATH "/dev/vsensor"

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

static int open_device(void)
{
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[ioctl_test] Cannot open %s: %s\n",
                DEVICE_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

/* ------------------------------------------------------------------ */
/*  VSENSOR_RESET                                                      */
/* ------------------------------------------------------------------ */

static void cmd_reset(void)
{
    int fd = open_device();

    if (ioctl(fd, VSENSOR_RESET) < 0) {
        fprintf(stderr, "[ioctl_test] VSENSOR_RESET failed: %s\n",
                strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("[ioctl_test] VSENSOR_RESET OK — ring buffer cleared.\n");
    printf("             Check dmesg for: \"[vsensor] ring buffer reset\"\n");
    close(fd);
}

/* ------------------------------------------------------------------ */
/*  VSENSOR_GET_STATS                                                  */
/* ------------------------------------------------------------------ */

static void cmd_stats(void)
{
    int fd = open_device();
    struct vsensor_stats stats;

    /* First do a few reads so the buffer has data */
    printf("[ioctl_test] Collecting 10 readings before stats...\n");
    for (int i = 0; i < 10; i++) {
        char buf[128] = {0};
        lseek(fd, 0, SEEK_SET);
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0)
            printf("  [%2d] %s", i + 1, buf);
    }

    if (ioctl(fd, VSENSOR_GET_STATS, &stats) < 0) {
        fprintf(stderr, "[ioctl_test] VSENSOR_GET_STATS failed: %s\n",
                strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("\n[ioctl_test] VSENSOR_GET_STATS results:\n");
    printf("  Sample count : %u\n",    stats.sample_count);
    printf("  Temperature  : min=%.1f°C  max=%.1f°C  avg=%.1f°C\n",
           stats.temp_min_x10 / 10.0f,
           stats.temp_max_x10 / 10.0f,
           stats.temp_avg_x10 / 10.0f);
    printf("  CPU Load     : min=%.1f%%  max=%.1f%%  avg=%.1f%%\n",
           stats.load_min_x10 / 10.0f,
           stats.load_max_x10 / 10.0f,
           stats.load_avg_x10 / 10.0f);

    close(fd);
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <reset|stats>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "reset") == 0) {
        cmd_reset();
    } else if (strcmp(argv[1], "stats") == 0) {
        cmd_stats();
    } else {
        fprintf(stderr, "[ioctl_test] Unknown command: %s\n", argv[1]);
        fprintf(stderr, "Usage: %s <reset|stats>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
