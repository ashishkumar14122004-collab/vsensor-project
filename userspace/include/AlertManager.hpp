/**
 * @file AlertManager.hpp
 * @brief Manages threshold-based alerts for sensor readings.
 */

#pragma once

/**
 * @brief Compares sensor values against configurable thresholds.
 *
 * Returns true if a value exceeds its threshold (alert condition).
 */
class AlertManager {
public:
    /**
     * @brief Construct with default thresholds.
     * @param temp_threshold Temperature alert threshold in °C (default 75.0).
     * @param load_threshold CPU load alert threshold in %  (default 90.0).
     */
    explicit AlertManager(float temp_threshold = 75.0f,
                          float load_threshold = 90.0f)
        : temp_threshold_(temp_threshold)
        , load_threshold_(load_threshold) {}

    /** @brief Return true if @p temp exceeds the temperature threshold. */
    bool checkTemp(float temp) const { return temp >= temp_threshold_; }

    /** @brief Return true if @p load exceeds the CPU load threshold. */
    bool checkLoad(float load) const { return load >= load_threshold_; }

    void setTempThreshold(float t) { temp_threshold_ = t; }
    void setLoadThreshold(float t) { load_threshold_ = t; }

    float getTempThreshold() const { return temp_threshold_; }
    float getLoadThreshold() const { return load_threshold_; }

private:
    float temp_threshold_;
    float load_threshold_;
};
