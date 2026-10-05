/**
 * @file test_demo_reader.cpp
 * @brief Unit tests for DemoReader.
 *
 * These tests run in any Linux environment (Codespaces, WSL2, VM, Docker).
 * No kernel module required.
 *
 * We cannot assert exact metric values because they vary by machine.
 * Instead we verify:
 *   - The read() call succeeds without crashing
 *   - Values are within plausible ranges OR are the -1 sentinel
 *   - The DemoSnapshot struct is correctly populated
 */

#include <gtest/gtest.h>
#include "DemoReader.hpp"

// ── Fixture ──────────────────────────────────────────────────────────────────

class DemoReaderTest : public ::testing::Test {
protected:
    DemoReader   reader;
    DemoSnapshot snap;

    void SetUp() override {
        snap = reader.read();
    }
};

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_F(DemoReaderTest, ReadDoesNotCrash) {
    // Simply calling read() and returning here is a pass.
    SUCCEED();
}

TEST_F(DemoReaderTest, CpuUsageIsValidOrUnavailable) {
    // Either -1 (unavailable) or a valid percentage [0, 100]
    if (snap.cpu_usage_pct >= 0.0f) {
        EXPECT_GE(snap.cpu_usage_pct,   0.0f);
        EXPECT_LE(snap.cpu_usage_pct, 100.0f);
    } else {
        EXPECT_FLOAT_EQ(snap.cpu_usage_pct, -1.0f);
    }
}

TEST_F(DemoReaderTest, MemTotalIsValidOrUnavailable) {
    if (snap.mem_total_mb >= 0) {
        // At least 64 MB — any reasonable Linux system has more
        EXPECT_GE(snap.mem_total_mb, 64L);
    } else {
        EXPECT_EQ(snap.mem_total_mb, -1L);
    }
}

TEST_F(DemoReaderTest, MemUsedDoesNotExceedTotal) {
    if (snap.mem_total_mb >= 0 && snap.mem_used_mb >= 0) {
        EXPECT_LE(snap.mem_used_mb, snap.mem_total_mb);
        EXPECT_GE(snap.mem_used_mb, 0L);
    }
}

TEST_F(DemoReaderTest, MemAvailableDoesNotExceedTotal) {
    if (snap.mem_total_mb >= 0 && snap.mem_available_mb >= 0) {
        EXPECT_LE(snap.mem_available_mb, snap.mem_total_mb);
        EXPECT_GE(snap.mem_available_mb, 0L);
    }
}

TEST_F(DemoReaderTest, ProcessCountIsValidOrUnavailable) {
    if (snap.process_count >= 0) {
        // Any live system has at least 1 process
        EXPECT_GE(snap.process_count, 1);
    } else {
        EXPECT_EQ(snap.process_count, -1);
    }
}

TEST_F(DemoReaderTest, UptimeIsValidOrUnavailable) {
    if (snap.uptime_seconds >= 0.0) {
        // Uptime must be positive
        EXPECT_GT(snap.uptime_seconds, 0.0);
    } else {
        EXPECT_DOUBLE_EQ(snap.uptime_seconds, -1.0);
    }
}

TEST_F(DemoReaderTest, MultipleReadsAreConsistent) {
    // Two successive reads should give consistent memory totals
    DemoSnapshot snap2 = reader.read();
    if (snap.mem_total_mb >= 0 && snap2.mem_total_mb >= 0) {
        // Total RAM does not change between reads
        EXPECT_EQ(snap.mem_total_mb, snap2.mem_total_mb);
    }
}
