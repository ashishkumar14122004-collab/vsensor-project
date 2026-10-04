/**
 * @file SensorParser.hpp
 * @brief Minimal JSON parser for vsensor device output.
 */

#pragma once

#include "SensorData.hpp"
#include <string>
#include <stdexcept>
#include <chrono>

/**
 * @brief Parses JSON strings produced by the vsensor kernel module.
 *
 * The expected format is:
 * @code
 *   {"temp":37.4,"load":62.1,"ts":1728043200000}
 * @endcode
 *
 * This is a hand-written parser to avoid external JSON library dependencies.
 */
class SensorParser {
public:
    /**
     * @brief Parse a raw JSON string into a SensorData struct.
     * @param raw Null-terminated JSON string from the device.
     * @return Populated SensorData.
     * @throws std::invalid_argument if parsing fails.
     */
    static SensorData parse(const std::string& raw) {
        SensorData data;

        float temp = extractFloat(raw, "temp");
        float load = extractFloat(raw, "load");
        long  ts   = extractLong(raw,  "ts");

        data.temperature = temp;
        data.cpu_load    = load;
        data.timestamp   = std::chrono::system_clock::time_point{
            std::chrono::milliseconds{ts}
        };

        return data;
    }

private:
    /**
     * @brief Extract a float value for @p key from a flat JSON string.
     */
    static float extractFloat(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos)
            throw std::invalid_argument("Key not found: " + key);
        pos += search.size();
        std::size_t end = json.find_first_of(",}", pos);
        return std::stof(json.substr(pos, end - pos));
    }

    /**
     * @brief Extract a long integer value for @p key from a flat JSON string.
     */
    static long extractLong(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos)
            throw std::invalid_argument("Key not found: " + key);
        pos += search.size();
        std::size_t end = json.find_first_of(",}", pos);
        return std::stol(json.substr(pos, end - pos));
    }
};
