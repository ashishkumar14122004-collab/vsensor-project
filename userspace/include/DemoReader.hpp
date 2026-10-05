/**
 * @file DemoReader.hpp
 * @brief Demo Mode system information reader.
 *
 * Reads real system metrics from /proc on any Linux container
 * (GitHub Codespaces, WSL2, Docker, VM) WITHOUT requiring the
 * vsensor kernel module to be loaded.
 *
 * This is a SEPARATE implementation — it does NOT touch the real
 * driver path (DeviceReader / /dev/vsensor / vsensor.ko).
 *
 * Metrics collected:
 *   - CPU usage    from /proc/stat
 *   - Memory info  from /proc/meminfo
 *   - Process count from /proc (directory entry count)
 *   - Uptime       from /proc/uptime
 */

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstdio>

/**
 * @brief Holds one snapshot of real system metrics for Demo Mode.
 */
struct DemoSnapshot {
    float  cpu_usage_pct   = -1.0f;  ///< CPU usage %; -1 if unavailable
    long   mem_total_mb    = -1;     ///< Total RAM in MB; -1 if unavailable
    long   mem_used_mb     = -1;     ///< Used  RAM in MB; -1 if unavailable
    long   mem_available_mb= -1;     ///< Available RAM in MB; -1 if unavailable
    int    process_count   = -1;     ///< Number of running processes; -1 if unavailable
    double uptime_seconds  = -1.0;   ///< System uptime in seconds; -1 if unavailable
};

/**
 * @brief Reads real system information from /proc for Demo Mode.
 *
 * All reads are done directly from the Linux /proc virtual filesystem.
 * No kernel module, no root privileges, no special permissions needed.
 *
 * Usage:
 * @code
 *   DemoReader reader;
 *   DemoSnapshot snap = reader.read();
 * @endcode
 */
class DemoReader {
public:
    /**
     * @brief Take one snapshot of all available system metrics.
     * @return DemoSnapshot with available values; -1 for unavailable fields.
     */
    DemoSnapshot read() {
        DemoSnapshot snap;
        snap.cpu_usage_pct    = readCpuUsage();
        readMemInfo(snap.mem_total_mb, snap.mem_used_mb, snap.mem_available_mb);
        snap.process_count    = readProcessCount();
        snap.uptime_seconds   = readUptime();
        return snap;
    }

private:
    /* ---------------------------------------------------------------- */
    /*  CPU usage from /proc/stat                                        */
    /* ---------------------------------------------------------------- */

    /**
     * @brief Read CPU usage percentage using two /proc/stat samples.
     *
     * Takes two readings 200 ms apart and computes usage from the delta.
     * Returns -1.0 if /proc/stat is unavailable.
     */
    float readCpuUsage() {
        long idle1 = 0, total1 = 0;
        long idle2 = 0, total2 = 0;

        if (!parseProcStat(idle1, total1))
            return -1.0f;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (!parseProcStat(idle2, total2))
            return -1.0f;

        long total_delta = total2 - total1;
        long idle_delta  = idle2  - idle1;

        if (total_delta <= 0)
            return 0.0f;

        float usage = 100.0f * (1.0f - static_cast<float>(idle_delta)
                                        / static_cast<float>(total_delta));
        return (usage < 0.0f) ? 0.0f : (usage > 100.0f ? 100.0f : usage);
    }

    /**
     * @brief Parse one line from /proc/stat for the aggregate CPU.
     * @param idle  Output: idle + iowait ticks.
     * @param total Output: sum of all CPU ticks.
     * @return true on success.
     */
    bool parseProcStat(long& idle, long& total) {
        std::ifstream f("/proc/stat");
        if (!f.is_open()) return false;

        std::string line;
        std::getline(f, line);           // first line: "cpu  ..."

        if (line.substr(0, 3) != "cpu") return false;

        std::istringstream ss(line.substr(5));
        // Fields: user nice system idle iowait irq softirq steal guest guest_nice
        long user = 0, nice = 0, system = 0, idle_t = 0,
             iowait = 0, irq = 0, softirq = 0, steal = 0;
        ss >> user >> nice >> system >> idle_t >> iowait
           >> irq >> softirq >> steal;

        idle  = idle_t + iowait;
        total = user + nice + system + idle_t + iowait + irq + softirq + steal;
        return true;
    }

    /* ---------------------------------------------------------------- */
    /*  Memory from /proc/meminfo                                        */
    /* ---------------------------------------------------------------- */

    /**
     * @brief Read MemTotal, MemAvailable from /proc/meminfo.
     *
     * MemUsed is computed as MemTotal - MemAvailable.
     * Values are returned in MB. Sets -1 on failure.
     */
    void readMemInfo(long& total_mb, long& used_mb, long& avail_mb) {
        std::ifstream f("/proc/meminfo");
        if (!f.is_open()) return;

        long mem_total_kb = -1;
        long mem_avail_kb = -1;

        std::string line;
        while (std::getline(f, line)) {
            long val = 0;
            if (sscanf(line.c_str(), "MemTotal: %ld kB", &val) == 1)
                mem_total_kb = val;
            else if (sscanf(line.c_str(), "MemAvailable: %ld kB", &val) == 1)
                mem_avail_kb = val;

            if (mem_total_kb != -1 && mem_avail_kb != -1)
                break;
        }

        if (mem_total_kb >= 0) total_mb = mem_total_kb / 1024;
        if (mem_avail_kb >= 0) avail_mb = mem_avail_kb / 1024;
        if (total_mb >= 0 && avail_mb >= 0)
            used_mb = total_mb - avail_mb;
    }

    /* ---------------------------------------------------------------- */
    /*  Process count from /proc                                         */
    /* ---------------------------------------------------------------- */

    /**
     * @brief Count numeric subdirectories in /proc (= running processes).
     * @return Process count, or -1 if /proc is unavailable.
     */
    int readProcessCount() {
        FILE* pipe = popen("ls -d /proc/[0-9]* 2>/dev/null | wc -l", "r");
        if (!pipe) return -1;
        int count = -1;
        fscanf(pipe, "%d", &count);
        pclose(pipe);
        return count;
    }

    /* ---------------------------------------------------------------- */
    /*  Uptime from /proc/uptime                                         */
    /* ---------------------------------------------------------------- */

    /**
     * @brief Read system uptime in seconds from /proc/uptime.
     * @return Uptime in seconds, or -1.0 on failure.
     */
    double readUptime() {
        std::ifstream f("/proc/uptime");
        if (!f.is_open()) return -1.0;
        double uptime = -1.0;
        f >> uptime;
        return uptime;
    }
};
