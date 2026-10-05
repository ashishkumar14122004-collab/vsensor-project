/**
 * @file AppConfig.hpp
 * @brief CLI argument parsing and application configuration.
 */

#pragma once

#include <string>
#include <iostream>
#include <cstring>

/**
 * @brief Holds all runtime configuration for vsensor_monitor.
 *
 * Populated from CLI arguments by AppConfig::parse().
 */
struct AppConfig {
    std::string device_path  = "/dev/vsensor"; ///< Path to the device file
    std::string logfile      = "vsensor.csv";  ///< CSV log output path
    int         interval_ms  = 1000;           ///< Poll interval in milliseconds
    float       temp_threshold = 75.0f;        ///< Temperature alert threshold (°C)
    float       load_threshold = 90.0f;        ///< CPU load alert threshold (%)
    bool        verbose      = false;          ///< Print raw JSON to stdout
    bool        demo_mode    = false;          ///< Run in Demo Mode (no kernel driver)

    /**
     * @brief Parse command-line arguments into this config.
     * @param argc Argument count.
     * @param argv Argument vector.
     * @return false if --help was requested or an error occurred.
     */
    bool parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                printHelp(argv[0]);
                return false;
            } else if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
                device_path = argv[++i];
            } else if (strcmp(argv[i], "--logfile") == 0 && i + 1 < argc) {
                logfile = argv[++i];
            } else if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
                interval_ms = std::stoi(argv[++i]);
            } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
                temp_threshold = std::stof(argv[++i]);
            } else if (strcmp(argv[i], "--verbose") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "--demo") == 0) {
                demo_mode = true;
            } else {
                std::cerr << "Unknown argument: " << argv[i] << "\n";
                printHelp(argv[0]);
                return false;
            }
        }
        return true;
    }

private:
    static void printHelp(const char* prog) {
        std::cout
            << "Usage: " << prog << " [options]\n\n"
            << "Options:\n"
            << "  --device   PATH   Device file path (default: /dev/vsensor)\n"
            << "  --logfile  PATH   CSV log output path (default: vsensor.csv)\n"
            << "  --interval MS     Poll interval in ms (default: 1000)\n"
            << "  --threshold TEMP  Temperature alert threshold °C (default: 75.0)\n"
            << "  --verbose         Print raw JSON to stdout\n"
            << "  --demo            Run in Demo Mode (no kernel driver required)\n"
            << "  --help            Show this help message\n";
    }
};
