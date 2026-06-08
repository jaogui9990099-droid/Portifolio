#include <nexus/memory_pool.hpp>
#include <gtest/gtest.h>
#include <vector>

using nexus::MemoryPool;

struct Node { int value; Node* next; double pad; };

TEST(MemoryPool, AllocateAndDeallocate) {
    MemoryPool<Node> pool(16);
    EXPECT_EQ(pool.capacity(), 16u);
    Node* n = pool.allocate();
    ASSERT_NE(n, nullptr);
    new (n) Node{42, nullptr, 0.0};
    EXPECT_EQ(n->value, 42);
    EXPECT_EQ(pool.in_use(), 1u);
    pool.deallocate(n);
    EXPECT_EQ(pool.in_use(), 0u);
}

TEST(MemoryPool, ExhaustsCapacity) {
    MemoryPool<Node> pool(4);
    std::vector<Node*> ptrs;
    for (int i = 0; i < 4; ++i) ptrs.push_back(pool.allocate());
    EXPECT_TRUE(pool.full());
    EXPECT_THROW(pool.allocate(), std::bad_alloc);
    pool.deallocate(ptrs[0]);
    EXPECT_FALSE(pool.full());
    Node* n = pool.allocate();
    EXPECT_NE(n, nullptr);
    for (std::size_t i = 1; i < ptrs.size(); ++i) pool.deallocate(ptrs[i]);
    pool.deallocate(n);
    EXPECT_TRUE(pool.empty());
}

TEST(MemoryPool, DeallocateNullptrIsNoop) {
    MemoryPool<Node> pool(4);
    EXPECT_NO_THROW(pool.deallocate(nullptr));
    EXPECT_EQ(pool.in_use(), 0u);
}

TEST(MemoryPool, ReusesSlots) {
    MemoryPool<Node> pool(2);
    Node* a = pool.allocate();
    Node* b = pool.allocate();
    pool.deallocate(a);
    pool.deallocate(b);
    Node* c = pool.allocate();
    Node* d = pool.allocate();
    EXPECT_NE(c, nullptr);
    EXPECT_NE(d, nullptr);
    EXPECT_NE(c, d);
    pool.deallocate(c);
    pool.deallocate(d);
}
