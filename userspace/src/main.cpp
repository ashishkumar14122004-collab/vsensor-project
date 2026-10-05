/**
 * @file main.cpp
 * @brief VSensor Monitor – entry point.
 *
 * Wires together:
 *   - AppConfig    (CLI argument parsing)
 *   - CsvLogger    (persistent log)
 *   - CircularBuffer + StatsEngine
 *   - DeviceReader  (background thread)
 *   - AlertManager
 *   - Dashboard    (ncurses UI, runs on main thread)
 *
 * Pass --demo to run in Demo Mode (no kernel driver required).
 *
 * The main thread drives the UI refresh loop at ~10 Hz, while the
 * DeviceReader thread polls /dev/vsensor at the configured interval.
 */

#include "AppConfig.hpp"
#include "CircularBuffer.hpp"
#include "SensorData.hpp"
#include "StatsEngine.hpp"
#include "CsvLogger.hpp"
#include "DeviceReader.hpp"
#include "AlertManager.hpp"
#include "Dashboard.hpp"

#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

/* Declared in demo_main.cpp */
int run_demo(int interval_ms);

/* ------------------------------------------------------------------ */
/*  Global shutdown flag (set by SIGINT handler)                       */
/* ------------------------------------------------------------------ */

static std::atomic<bool> g_shutdown{false};

static void onSignal(int /*signum*/) {
    g_shutdown = true;
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char* argv[])
{
    /* Parse configuration */
    AppConfig config;
    if (!config.parse(argc, argv))
        return 0;

    /* ── Demo Mode: no kernel driver required ─────────────────────── */
    if (config.demo_mode) {
        return run_demo(config.interval_ms);
    }
    /* ─────────────────────────────────────────────────────────────── */

    /* Install signal handler */
    std::signal(SIGINT,  onSignal);
    std::signal(SIGTERM, onSignal);

    /* Shared data structures */
    CircularBuffer<SensorData> buffer(256);
    CsvLogger                  logger;
    StatsEngine                stats(buffer);
    AlertManager               alerts(config.temp_threshold,
                                      config.load_threshold);

    /* Open log file */
    if (!logger.open(config.logfile)) {
        std::cerr << "[main] Failed to open log file: "
                  << config.logfile << "\n";
        return 1;
    }
    std::cout << "[main] Logging to: " << config.logfile << "\n";

    /* Start reader thread */
    DeviceReader reader(config, buffer, logger);
    reader.start();

    /* Initialize dashboard */
    Dashboard dashboard(buffer, stats, alerts, config);
    dashboard.init();

    /* Main UI loop – refresh at ~10 Hz */
    while (!g_shutdown && reader.isRunning()) {
        if (!dashboard.render())   // returns false when user presses 'q'
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    /* Graceful shutdown */
    dashboard.cleanup();
    reader.stop();
    logger.close();

    std::cout << "[main] VSensor monitor exited cleanly.\n";
    return 0;
}
