/**
 * @file test_sensor_parser.cpp
 * @brief Unit tests for SensorParser.
 */

#include <gtest/gtest.h>
#include "SensorParser.hpp"
#include <cmath>

TEST(SensorParserTest, ParsesValidJson) {
    std::string raw = R"({"temp":37.4,"load":62.1,"ts":1728043200000})";
    SensorData d = SensorParser::parse(raw);
    EXPECT_NEAR(d.temperature, 37.4f, 0.01f);
    EXPECT_NEAR(d.cpu_load,    62.1f, 0.01f);
}

TEST(SensorParserTest, ParsesMinBoundaryValues) {
    std::string raw = R"({"temp":20.0,"load":0.0,"ts":1000})";
    SensorData d = SensorParser::parse(raw);
    EXPECT_NEAR(d.temperature, 20.0f, 0.01f);
    EXPECT_NEAR(d.cpu_load,     0.0f, 0.01f);
}

TEST(SensorParserTest, ParsesMaxBoundaryValues) {
    std::string raw = R"({"temp":85.0,"load":100.0,"ts":9999999999000})";
    SensorData d = SensorParser::parse(raw);
    EXPECT_NEAR(d.temperature, 85.0f,  0.01f);
    EXPECT_NEAR(d.cpu_load,   100.0f,  0.01f);
}

TEST(SensorParserTest, ThrowsOnMissingKey) {
    std::string raw = R"({"load":62.1,"ts":1000})"; // missing "temp"
    EXPECT_THROW(SensorParser::parse(raw), std::invalid_argument);
}

TEST(SensorParserTest, ThrowsOnEmptyString) {
    EXPECT_THROW(SensorParser::parse(""), std::invalid_argument);
}

TEST(SensorParserTest, TimestampIsSet) {
    std::string raw = R"({"temp":25.0,"load":50.0,"ts":1728043200000})";
    SensorData d = SensorParser::parse(raw);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        d.timestamp.time_since_epoch()).count();
    EXPECT_EQ(ms, 1728043200000LL);
}
