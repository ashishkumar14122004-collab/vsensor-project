/**
 * @file demo_main.cpp
 * @brief VSensor Monitor – Demo Mode entry point.
 *
 * Runs when the user passes --demo flag.
 * Uses DemoReader to collect REAL system metrics from /proc.
 * Does NOT load or use the vsensor kernel module.
 * Does NOT use ncurses — plain terminal output for maximum portability.
 *
 * Usage:
 *   ./vsensor_monitor --demo
 *   ./vsensor_monitor --demo --interval 2000
 */

#include "DemoReader.hpp"
#include "AppConfig.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <cmath>

static std::atomic<bool> g_stop{false};

static void onSignal(int) { g_stop = true; }

/* ------------------------------------------------------------------ */
/*  Formatting helpers                                                 */
/* ------------------------------------------------------------------ */

static std::string fmtFloat(float val, const std::string& unit,
                              const std::string& fallback = "N/A") {
    if (val < 0.0f) return fallback;
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << val << " " << unit;
    return ss.str();
}

static std::string fmtLong(long val, const std::string& unit,
                             const std::string& fallback = "N/A") {
    if (val < 0) return fallback;
    return std::to_string(val) + " " + unit;
}

static std::string fmtUptime(double seconds) {
    if (seconds < 0.0) return "N/A";
    int s = static_cast<int>(seconds);
    int days  = s / 86400; s %= 86400;
    int hours = s / 3600;  s %= 3600;
    int mins  = s / 60;    s %= 60;
    std::ostringstream ss;
    if (days)  ss << days  << "d ";
    if (hours) ss << hours << "h ";
    if (mins)  ss << mins  << "m ";
    ss << s << "s";
    return ss.str();
}

static std::string cpuBar(float pct, int width = 30) {
    if (pct < 0.0f) return std::string(width, '?');
    int filled = static_cast<int>((pct / 100.0f) * width);
    filled = std::max(0, std::min(width, filled));
    return std::string(filled, '#') + std::string(width - filled, '-');
}

/* ------------------------------------------------------------------ */
/*  Banner                                                             */
/* ------------------------------------------------------------------ */

static void printBanner(int sample) {
    std::cout << "\n";
    std::cout << "==========================================\n";
    std::cout << "   LINUX SYSTEM RESOURCE MONITOR\n";
    std::cout << "==========================================\n";
    std::cout << "   Mode: DEMO MODE  (sample #" << sample << ")\n";
    std::cout << "   DEMO MODE - Kernel driver is not loaded\n";
    std::cout << "==========================================\n";
}

static void printSnapshot(const DemoSnapshot& snap) {
    std::cout << "\n";

    // CPU
    std::string cpu_str  = fmtFloat(snap.cpu_usage_pct, "%");
    std::string cpu_bar  = "[" + cpuBar(snap.cpu_usage_pct) + "]";
    std::cout << "  CPU Usage        : " << std::left << std::setw(10)
              << cpu_str << "  " << cpu_bar << "\n";

    // Memory
    std::cout << "  Memory Total     : " << fmtLong(snap.mem_total_mb,     "MB") << "\n";
    std::cout << "  Memory Used      : " << fmtLong(snap.mem_used_mb,      "MB") << "\n";
    std::cout << "  Memory Available : " << fmtLong(snap.mem_available_mb, "MB") << "\n";

    // Processes & uptime
    std::cout << "  Processes        : "
              << (snap.process_count >= 0
                  ? std::to_string(snap.process_count) : "N/A") << "\n";
    std::cout << "  Uptime           : " << fmtUptime(snap.uptime_seconds) << "\n";

    std::cout << "\n";
    std::cout << "------------------------------------------\n";
    std::cout << "  Driver : DEMO / NOT LOADED\n";
    std::cout << "  Device : /dev/vsensor  (unavailable)\n";
    std::cout << "  Source : /proc/stat, /proc/meminfo,\n";
    std::cout << "           /proc/uptime, /proc/[PID]\n";
    std::cout << "------------------------------------------\n";
    std::cout << "  Press Ctrl+C to exit\n";
}

/* ------------------------------------------------------------------ */
/*  run_demo  – called from main.cpp when --demo flag is set           */
/* ------------------------------------------------------------------ */

/**
 * @brief Entry point for Demo Mode.
 *
 * Called by main() when config.demo_mode == true.
 * Loops at the configured interval until SIGINT / SIGTERM.
 *
 * @param interval_ms  Refresh interval (from --interval, default 1000 ms).
 * @return 0 on clean exit.
 */
int run_demo(int interval_ms) {
    std::signal(SIGINT,  onSignal);
    std::signal(SIGTERM, onSignal);

    DemoReader reader;
    int sample = 0;

    while (!g_stop) {
        ++sample;
        DemoSnapshot snap = reader.read();

        // Clear terminal for a clean refresh
        std::cout << "\033[2J\033[H";   // ANSI clear screen + home

        printBanner(sample);
        printSnapshot(snap);

        // Wait for next interval (check g_stop every 100 ms)
        int waited = 0;
        while (!g_stop && waited < interval_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            waited += 100;
        }
    }

    std::cout << "\n[demo] Exited cleanly.\n";
    return 0;
}
