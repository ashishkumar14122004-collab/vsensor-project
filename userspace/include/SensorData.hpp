/**
 * @file SensorData.hpp
 * @brief Plain data structure representing one sensor reading.
 */

#pragma once

#include <chrono>

/**
 * @brief A single parsed sensor reading from /dev/vsensor.
 */
struct SensorData {
    std::chrono::system_clock::time_point timestamp; ///< When the reading was taken
    float temperature = 0.0f;  ///< Temperature in degrees Celsius
    float cpu_load    = 0.0f;  ///< CPU load as a percentage (0–100)
};
