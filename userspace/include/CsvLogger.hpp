/**
 * @file CsvLogger.hpp
 * @brief Thread-safe CSV logger for sensor data.
 */

#pragma once

#include "SensorData.hpp"
#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

/**
 * @brief Writes SensorData readings to a CSV file in a thread-safe manner.
 *
 * CSV format:
 * @code
 *   timestamp_iso,temperature_c,cpu_load_pct
 *   2026-10-04T12:00:00.000,37.4,62.1
 * @endcode
 */
class CsvLogger {
public:
    CsvLogger() = default;
    ~CsvLogger() { close(); }

    /**
     * @brief Open (or create) the CSV log file.
     * @param path File path for the log.
     * @return true on success, false on failure.
     */
    bool open(const std::string& path) {
        std::lock_guard<std::mutex> lk(mtx_);
        file_.open(path, std::ios::app);
        if (!file_.is_open())
            return false;
        // Write header only if file is empty
        file_.seekp(0, std::ios::end);
        if (file_.tellp() == 0)
            file_ << "timestamp_iso,temperature_c,cpu_load_pct\n";
        return true;
    }

    /**
     * @brief Log a single SensorData reading.
     * @param data The reading to write.
     */
    void log(const SensorData& data) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (!file_.is_open())
            return;
        file_ << formatTimestamp(data.timestamp) << ","
              << data.temperature << ","
              << data.cpu_load << "\n";
        file_.flush();
    }

    /** @brief Close the log file. */
    void close() {
        std::lock_guard<std::mutex> lk(mtx_);
        if (file_.is_open())
            file_.close();
    }

private:
    std::ofstream file_;
    std::mutex    mtx_;

    static std::string formatTimestamp(
        const std::chrono::system_clock::time_point& tp)
    {
        auto tt = std::chrono::system_clock::to_time_t(tp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      tp.time_since_epoch()) % 1000;
        std::ostringstream ss;
        ss << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%S")
           << "." << std::setw(3) << std::setfill('0') << ms.count();
        return ss.str();
    }
};
