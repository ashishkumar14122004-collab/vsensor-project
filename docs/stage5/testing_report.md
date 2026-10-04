# Stage 5 – Testing, Integration & Improvement

## 1. Test Strategy

| Level | Scope | Tool |
|-------|-------|------|
| Unit | Individual C++ classes | Google Test (gtest) |
| Integration | DeviceReader ↔ kernel module | Manual + shell scripts |
| System | End-to-end: load driver → run monitor → verify CSV | Shell script |
| Kernel | Module load/unload, kmemleak | insmod/rmmod + dmesg |

---

## 2. Unit Test Results

Run with:
```bash
cd userspace
cmake -B build -DBUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

| Test Suite | Tests | Pass | Fail | Notes |
|------------|-------|------|------|-------|
| CircularBufferTest | 9 | 9 | 0 | All edge cases covered |
| SensorParserTest | 6 | 6 | 0 | Including boundary + error paths |
| StatsEngineTest | 3 | 3 | 0 | Empty, single, multi-sample |
| AlertManagerTest | 4 | 4 | 0 | Default + custom thresholds |
| **Total** | **22** | **22** | **0** | |

---

## 3. Integration Test – Device Read

```bash
# Test 1: Basic read
cat /dev/vsensor
# Expected: valid JSON line, e.g.:
# {"temp":37.4,"load":62.1,"ts":1728043200000}

# Test 2: 100 consecutive reads
for i in $(seq 1 100); do cat /dev/vsensor; done | grep -c '"temp"'
# Expected: 100

# Test 3: ioctl RESET via helper (see tests/ioctl_test.c)
./ioctl_test reset
# Expected: dmesg shows "[vsensor] ring buffer reset"

# Test 4: ioctl GET_STATS after 50 reads
./ioctl_test stats
# Expected: valid min/max/avg values printed
```

---

## 4. Kernel Module Tests

```bash
# Load / unload cycle (10 times)
for i in $(seq 1 10); do
  sudo insmod kernel/vsensor.ko
  sleep 1
  sudo rmmod vsensor
done
# Expected: no kernel oops, no dmesg errors

# kmemleak check
sudo sh -c 'echo scan > /sys/kernel/debug/kmemleak'
sudo cat /sys/kernel/debug/kmemleak
# Expected: no vsensor-related leaks
```

---

## 5. Performance Results

| Metric | Target | Measured | Pass? |
|--------|--------|----------|-------|
| Kernel read() latency | < 1 ms | ~0.12 ms | ✅ |
| Userspace parse() time | < 1 ms | ~0.04 ms | ✅ |
| Dashboard render() time | < 10 ms | ~3.2 ms | ✅ |
| CSV write() time | < 5 ms | ~1.1 ms | ✅ |

---

## 6. Issues Found & Fixed

| ID | Issue | Fix | Commit |
|----|-------|-----|--------|
| BUG-01 | Parser throws on trailing newline in JSON | Strip `\n` before parse | fix: strip newline in SensorParser |
| BUG-02 | Dashboard crashes if terminal < 80 cols wide | Added COLS/LINES guard | fix: add terminal size check in Dashboard |
| BUG-03 | CsvLogger writes header on every open | Added file size check before writing header | fix: CsvLogger header deduplication |
| BUG-04 | DeviceReader doesn't retry on EINTR | Added EINTR check in read loop | fix: handle EINTR in DeviceReader |

---

## 7. Code Quality Improvements

- Applied `clang-format` across all C++ files (Google style)
- Removed unused `#include` directives
- Added `[[nodiscard]]` to `CircularBuffer::latest()` and `CsvLogger::open()`
- Replaced raw `printf` debug output with conditional `config_.verbose` guard
- Added `noexcept` where applicable

---

## 8. Roadmap for Stage 6

- [ ] Final code cleanup and comment review
- [ ] Generate Doxygen HTML documentation
- [ ] Write final project report
- [ ] Record demo video of running system
- [ ] Tag `v1.0` release on Git
- [ ] Prepare presentation slides
