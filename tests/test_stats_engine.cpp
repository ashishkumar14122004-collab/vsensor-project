/**
 * @file test_stats_engine.cpp
 * @brief Unit tests for StatsEngine.
 */

#include <gtest/gtest.h>
#include "StatsEngine.hpp"
#include "CircularBuffer.hpp"
#include "SensorData.hpp"
#include <chrono>

static SensorData makeSample(float temp, float load) {
    SensorData d;
    d.timestamp   = std::chrono::system_clock::now();
    d.temperature = temp;
    d.cpu_load    = load;
    return d;
}

class StatsEngineTest : public ::testing::Test {
protected:
    CircularBuffer<SensorData> buf{16};
    StatsEngine                engine{buf};
};

TEST_F(StatsEngineTest, ReturnsZerosOnEmptyBuffer) {
    EXPECT_FLOAT_EQ(engine.minTemp(), 0.0f);
    EXPECT_FLOAT_EQ(engine.maxTemp(), 0.0f);
    EXPECT_FLOAT_EQ(engine.avgTemp(), 0.0f);
}

TEST_F(StatsEngineTest, SingleSampleStatistics) {
    buf.push(makeSample(42.0f, 55.0f));
    EXPECT_FLOAT_EQ(engine.minTemp(), 42.0f);
    EXPECT_FLOAT_EQ(engine.maxTemp(), 42.0f);
    EXPECT_FLOAT_EQ(engine.avgTemp(), 42.0f);
    EXPECT_FLOAT_EQ(engine.minLoad(), 55.0f);
    EXPECT_FLOAT_EQ(engine.maxLoad(), 55.0f);
    EXPECT_FLOAT_EQ(engine.avgLoad(), 55.0f);
}

TEST_F(StatsEngineTest, MultiSampleMinMaxAvg) {
    buf.push(makeSample(20.0f, 10.0f));
    buf.push(makeSample(50.0f, 50.0f));
    buf.push(makeSample(80.0f, 90.0f));
    EXPECT_FLOAT_EQ(engine.minTemp(), 20.0f);
    EXPECT_FLOAT_EQ(engine.maxTemp(), 80.0f);
    EXPECT_NEAR(engine.avgTemp(), 50.0f, 0.01f);
    EXPECT_FLOAT_EQ(engine.minLoad(), 10.0f);
    EXPECT_FLOAT_EQ(engine.maxLoad(), 90.0f);
    EXPECT_NEAR(engine.avgLoad(), 50.0f, 0.01f);
}
