/**
 * @file StatsEngine.hpp
 * @brief Computes min, max, and average statistics over a CircularBuffer.
 */

#pragma once

#include "CircularBuffer.hpp"
#include "SensorData.hpp"
#include <limits>
#include <numeric>

/**
 * @brief Computes real-time statistics over buffered SensorData readings.
 *
 * Reads from a shared CircularBuffer<SensorData>. Statistics are recomputed
 * on every call (suitable for the moderate update rates used here).
 */
class StatsEngine {
public:
    /**
     * @brief Construct a StatsEngine bound to the given buffer.
     * @param buffer Reference to the shared sensor data buffer.
     */
    explicit StatsEngine(const CircularBuffer<SensorData>& buffer)
        : buffer_(buffer) {}

    /** @brief Minimum temperature across all buffered readings. */
    float minTemp() const { return compute([](const SensorData& d){ return d.temperature; }).first; }

    /** @brief Maximum temperature across all buffered readings. */
    float maxTemp() const { return compute([](const SensorData& d){ return d.temperature; }).second; }

    /** @brief Average temperature across all buffered readings. */
    float avgTemp() const { return average([](const SensorData& d){ return d.temperature; }); }

    /** @brief Minimum CPU load across all buffered readings. */
    float minLoad() const { return compute([](const SensorData& d){ return d.cpu_load; }).first; }

    /** @brief Maximum CPU load across all buffered readings. */
    float maxLoad() const { return compute([](const SensorData& d){ return d.cpu_load; }).second; }

    /** @brief Average CPU load across all buffered readings. */
    float avgLoad() const { return average([](const SensorData& d){ return d.cpu_load; }); }

private:
    const CircularBuffer<SensorData>& buffer_;

    template <typename Selector>
    std::pair<float, float> compute(Selector sel) const {
        auto readings = buffer_.range(buffer_.size());
        if (readings.empty())
            return {0.0f, 0.0f};
        float lo = sel(readings[0]);
        float hi = lo;
        for (const auto& r : readings) {
            float v = sel(r);
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
        return {lo, hi};
    }

    template <typename Selector>
    float average(Selector sel) const {
        auto readings = buffer_.range(buffer_.size());
        if (readings.empty())
            return 0.0f;
        float sum = 0.0f;
        for (const auto& r : readings)
            sum += sel(r);
        return sum / static_cast<float>(readings.size());
    }
};
