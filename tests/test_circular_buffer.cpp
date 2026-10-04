/**
 * @file test_circular_buffer.cpp
 * @brief Unit tests for CircularBuffer<T>.
 */

#include <gtest/gtest.h>
#include "CircularBuffer.hpp"

// -------------------------------------------------------------------

TEST(CircularBufferTest, StartsEmpty) {
    CircularBuffer<int> buf(4);
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);
}

TEST(CircularBufferTest, PushAndSize) {
    CircularBuffer<int> buf(4);
    buf.push(1);
    buf.push(2);
    EXPECT_EQ(buf.size(), 2u);
    EXPECT_FALSE(buf.empty());
}

TEST(CircularBufferTest, LatestReturnsLastPushed) {
    CircularBuffer<int> buf(4);
    buf.push(10);
    buf.push(20);
    buf.push(30);
    EXPECT_EQ(buf.latest(), 30);
}

TEST(CircularBufferTest, OverwritesOldestWhenFull) {
    CircularBuffer<int> buf(3);
    buf.push(1);
    buf.push(2);
    buf.push(3); // full
    buf.push(4); // overwrites 1
    EXPECT_EQ(buf.size(), 3u);
    auto v = buf.range(3);
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v[1], 3);
    EXPECT_EQ(v[2], 4);
}

TEST(CircularBufferTest, RangeReturnsChrononlogicalOrder) {
    CircularBuffer<int> buf(8);
    for (int i = 1; i <= 5; ++i)
        buf.push(i);
    auto v = buf.range(5);
    ASSERT_EQ(v.size(), 5u);
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(v[i], i + 1);
}

TEST(CircularBufferTest, RangeClampsToAvailable) {
    CircularBuffer<int> buf(8);
    buf.push(42);
    auto v = buf.range(100); // request more than available
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 42);
}

TEST(CircularBufferTest, LatestThrowsWhenEmpty) {
    CircularBuffer<int> buf(4);
    EXPECT_THROW(buf.latest(), std::underflow_error);
}

TEST(CircularBufferTest, ClearResetsBuffer) {
    CircularBuffer<int> buf(4);
    buf.push(1);
    buf.push(2);
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);
}

TEST(CircularBufferTest, FullFlag) {
    CircularBuffer<int> buf(3);
    EXPECT_FALSE(buf.full());
    buf.push(1); buf.push(2); buf.push(3);
    EXPECT_TRUE(buf.full());
}
