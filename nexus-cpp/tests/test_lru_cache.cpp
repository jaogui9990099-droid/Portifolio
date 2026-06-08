#include <nexus/lru_cache.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using nexus::LruCache;
using namespace std::chrono_literals;

TEST(LruCache, PutAndGet) {
    LruCache<std::string, int> cache(10);
    cache.put("a", 1); cache.put("b", 2);
    EXPECT_EQ(cache.get("a"), 1);
    EXPECT_EQ(cache.get("b"), 2);
    EXPECT_FALSE(cache.get("c").has_value());
}

TEST(LruCache, EvictsLruEntry) {
    LruCache<int, int> cache(3);
    cache.put(1, 1); cache.put(2, 2); cache.put(3, 3);
    cache.get(1);
    cache.put(4, 4);
    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_TRUE(cache.get(1).has_value());
    EXPECT_TRUE(cache.get(3).has_value());
    EXPECT_TRUE(cache.get(4).has_value());
}

TEST(LruCache, UpdateMovesToFront) {
    LruCache<int, int> cache(2);
    cache.put(1, 10); cache.put(2, 20);
    cache.put(1, 99);
    cache.put(3, 30);
    EXPECT_EQ(cache.get(1), 99);
    EXPECT_FALSE(cache.get(2).has_value());
}

TEST(LruCache, Erase) {
    LruCache<std::string, std::string> cache(5);
    cache.put("k", "v");
    EXPECT_TRUE(cache.erase("k"));
    EXPECT_FALSE(cache.get("k").has_value());
    EXPECT_FALSE(cache.erase("k"));
}

TEST(LruCache, SizeAndClear) {
    LruCache<int, int> cache(10);
    for (int i = 0; i < 5; ++i) cache.put(i, i);
    EXPECT_EQ(cache.size(), 5u);
    cache.clear();
    EXPECT_EQ(cache.size(), 0u);
    EXPECT_TRUE(cache.empty());
}

TEST(LruCache, TtlExpiry) {
    using std::chrono::duration;
    LruCache<std::string, int> cache(10, duration<double>(0.05));
    cache.put("x", 42);
    EXPECT_EQ(cache.get("x"), 42);
    std::this_thread::sleep_for(80ms);
    EXPECT_FALSE(cache.get("x").has_value());
    EXPECT_EQ(cache.stats().expirations, 1u);
}

TEST(LruCache, StatsAccuracy) {
    LruCache<int, int> cache(5);
    cache.put(1, 1);
    cache.get(1);
    cache.get(2);
    auto s = cache.stats();
    EXPECT_EQ(s.hits, 1u);
    EXPECT_EQ(s.misses, 1u);
    EXPECT_GT(s.hit_rate(), 0.0);
}

TEST(LruCache, ConcurrentAccess) {
    LruCache<int, int> cache(256);
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < 5000; ++i) {
                int key = (t * 5000 + i) % 512;
                cache.put(key, key);
                cache.get(key);
            }
        });
    }
    for (auto& th : threads) th.join();
    EXPECT_LE(cache.size(), 256u);
}
