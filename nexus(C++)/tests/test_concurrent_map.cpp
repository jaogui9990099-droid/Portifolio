#include <nexus/concurrent_map.hpp>

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>

using nexus::ConcurrentMap;

TEST(ConcurrentMap, PutAndGet) {
    ConcurrentMap<std::string, int> map;
    map.put("a", 1);
    map.put("b", 2);

    EXPECT_EQ(map.get("a"), 1);
    EXPECT_EQ(map.get("b"), 2);
    EXPECT_FALSE(map.get("c").has_value());
}

TEST(ConcurrentMap, OverwriteExistingKey) {
    ConcurrentMap<int, int> map;
    map.put(1, 10);
    map.put(1, 20);
    EXPECT_EQ(map.get(1), 20);
    EXPECT_EQ(map.size(), 1u);
}

TEST(ConcurrentMap, Remove) {
    ConcurrentMap<std::string, std::string> map;
    map.put("x", "hello");
    EXPECT_TRUE(map.remove("x"));
    EXPECT_FALSE(map.get("x").has_value());
    EXPECT_EQ(map.size(), 0u);
}

TEST(ConcurrentMap, RemoveNonExistent) {
    ConcurrentMap<int, int> map;
    EXPECT_FALSE(map.remove(42));
}

TEST(ConcurrentMap, SizeTracking) {
    ConcurrentMap<int, int> map;
    EXPECT_EQ(map.size(), 0u);
    map.put(1, 1);
    map.put(2, 2);
    EXPECT_EQ(map.size(), 2u);
    map.remove(1);
    EXPECT_EQ(map.size(), 1u);
}

TEST(ConcurrentMap, GrowsBeyondInitialCapacity) {
    ConcurrentMap<int, int> map(8);
    for (int i = 0; i < 1000; ++i) {
        map.put(i, i * 2);
    }
    EXPECT_EQ(map.size(), 1000u);
    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(map.get(i), i * 2) << "key=" << i;
    }
}

TEST(ConcurrentMap, ConcurrentWrites) {
    ConcurrentMap<int, int> map(256);
    constexpr int kThreads = 8;
    constexpr int kPerThread = 1000;
    std::vector<std::thread> threads;

    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            int base = t * kPerThread;
            for (int i = 0; i < kPerThread; ++i) {
                map.put(base + i, base + i);
            }
        });
    }
    for (auto& th : threads) th.join();

    EXPECT_EQ(map.size(), static_cast<std::size_t>(kThreads * kPerThread));
}

TEST(ConcurrentMap, ConcurrentReadsAndWrites) {
    ConcurrentMap<int, int> map(512);
    constexpr int kEntries = 500;
    for (int i = 0; i < kEntries; ++i) map.put(i, i);

    std::atomic<bool> stop{false};
    std::vector<std::thread> readers;

    for (int t = 0; t < 4; ++t) {
        readers.emplace_back([&] {
            while (!stop.load(std::memory_order_relaxed)) {
                for (int i = 0; i < kEntries; ++i) {
                    auto v = map.get(i);
                    if (v) EXPECT_EQ(*v, i);
                }
            }
        });
    }

    for (int i = kEntries; i < kEntries * 2; ++i) map.put(i, i);
    stop.store(true);
    for (auto& r : readers) r.join();
}

TEST(ConcurrentMap, ForEach) {
    ConcurrentMap<int, int> map;
    map.put(1, 10);
    map.put(2, 20);
    map.put(3, 30);

    int sum = 0;
    map.for_each([&](int, int v) { sum += v; });
    EXPECT_EQ(sum, 60);
}
