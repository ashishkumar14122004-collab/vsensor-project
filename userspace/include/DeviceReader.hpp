/**
 * @file DeviceReader.hpp
 * @brief Background thread that reads from /dev/vsensor.
 */

#pragma once

#include "AppConfig.hpp"
#include "CircularBuffer.hpp"
#include "SensorData.hpp"
#include "SensorParser.hpp"
#include "CsvLogger.hpp"

#include <thread>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

/**
 * @brief Spawns a background reader thread that polls /dev/vsensor.
 *
 * On each successful read:
 *   1. Parses the JSON string with SensorParser.
 *   2. Pushes the SensorData into the shared CircularBuffer.
 *   3. Logs the reading via CsvLogger.
 *
 * If the device is unavailable, the thread retries with exponential
 * backoff (up to 5 attempts before giving up).
 */
class DeviceReader {
public:
    /**
     * @brief Construct a DeviceReader.
     * @param config  Application configuration (device path, interval).
     * @param buffer  Shared circular buffer for sensor data.
     * @param logger  CSV logger instance.
     */
    DeviceReader(const AppConfig&           config,
                 CircularBuffer<SensorData>& buffer,
                 CsvLogger&                  logger)
        : config_(config), buffer_(buffer), logger_(logger) {}

    ~DeviceReader() { stop(); }

    /** @brief Start the background reader thread. */
    void start() {
        running_ = true;
        thread_ = std::thread(&DeviceReader::run, this);
    }

    /** @brief Signal the reader thread to stop and wait for it to finish. */
    void stop() {
        running_ = false;
        if (thread_.joinable())
            thread_.join();
    }

    /** @brief Return true if the reader is actively reading from the device. */
    bool isRunning() const { return running_.load(); }

private:
    const AppConfig&            config_;
    CircularBuffer<SensorData>& buffer_;
    CsvLogger&                  logger_;
    std::thread                 thread_;
    std::atomic<bool>           running_{false};

    static constexpr int    MAX_RETRIES  = 5;
    static constexpr char   READ_BUF_SZ  = 127;

    void run() {
        int  fd         = -1;
        int  retries    = 0;
        int  backoff_ms = 500;

        while (running_) {
            /* --- Open device (with retry) --- */
            if (fd < 0) {
                fd = open(config_.device_path.c_str(), O_RDONLY);
                if (fd < 0) {
                    std::cerr << "[DeviceReader] Cannot open "
                              << config_.device_path
                              << ": " << strerror(errno) << "\n";
                    if (++retries > MAX_RETRIES) {
                        std::cerr << "[DeviceReader] Max retries exceeded. Stopping.\n";
                        running_ = false;
                        return;
                    }
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(backoff_ms));
                    backoff_ms = std::min(backoff_ms * 2, 8000);
                    continue;
                }
                retries    = 0;
                backoff_ms = 500;
            }

            /* --- Read one sample --- */
            char buf[128] = {};
            ssize_t n = read(fd, buf, sizeof(buf) - 1);

            if (n <= 0) {
                std::cerr << "[DeviceReader] read() failed: " << strerror(errno) << "\n";
                close(fd);
                fd = -1;
                continue;
            }

            std::string raw(buf, static_cast<std::size_t>(n));

            if (config_.verbose)
                std::cout << "[raw] " << raw;

            try {
                SensorData data = SensorParser::parse(raw);
                buffer_.push(data);
                logger_.log(data);
            } catch (const std::exception& e) {
                std::cerr << "[DeviceReader] Parse error: " << e.what()
                          << " | raw=" << raw << "\n";
            }

            /* --- Seek back to re-read on next iteration --- */
            lseek(fd, 0, SEEK_SET);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(config_.interval_ms));
        }

        if (fd >= 0)
            close(fd);
    }
};
