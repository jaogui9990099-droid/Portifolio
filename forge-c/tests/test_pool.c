#include "unity.h"
#include "forge/pool.h"
#include <stdlib.h>

void test_pool_run_all(void);

static void test_alloc_free(void) {
    TEST_BEGIN("pool/alloc_free");
    forge_pool_t pool;
    ASSERT_EQ(forge_pool_init(&pool, 64, 16), 0);
    void* a = forge_pool_alloc(&pool);
    void* b = forge_pool_alloc(&pool);
    ASSERT_NOTNULL(a);
    ASSERT_NOTNULL(b);
    ASSERT_NE(a, b);
    ASSERT_EQ(forge_pool_stats(&pool).in_use, 2u);
    forge_pool_free(&pool, a);
    ASSERT_EQ(forge_pool_stats(&pool).in_use, 1u);
    forge_pool_free(&pool, b);
    ASSERT_EQ(forge_pool_stats(&pool).in_use, 0u);
    forge_pool_destroy(&pool);
}

static void test_exhaustion_returns_null(void) {
    TEST_BEGIN("pool/exhaustion_returns_null");
    forge_pool_t pool;
    forge_pool_init(&pool, 64, 4);
    void* ptrs[4];
    for (int i = 0; i < 4; ++i) ptrs[i] = forge_pool_alloc(&pool);
    ASSERT_NULL(forge_pool_alloc(&pool));
    for (int i = 0; i < 4; ++i) forge_pool_free(&pool, ptrs[i]);
    forge_pool_destroy(&pool);
}

static void test_peak_tracking(void) {
    TEST_BEGIN("pool/peak_tracking");
    forge_pool_t pool;
    forge_pool_init(&pool, 64, 8);
    void* a = forge_pool_alloc(&pool);
    void* b = forge_pool_alloc(&pool);
    void* c = forge_pool_alloc(&pool);
    ASSERT_EQ(forge_pool_stats(&pool).peak, 3u);
    forge_pool_free(&pool, c);
    ASSERT_EQ(forge_pool_stats(&pool).peak, 3u);
    forge_pool_free(&pool, a);
    forge_pool_free(&pool, b);
    forge_pool_destroy(&pool);
}

static void test_slot_reuse(void) {
    TEST_BEGIN("pool/slot_reuse");
    forge_pool_t pool;
    forge_pool_init(&pool, 64, 2);
    void* a = forge_pool_alloc(&pool);
    forge_pool_free(&pool, a);
    void* b = forge_pool_alloc(&pool);
    ASSERT_EQ(a, b);
    forge_pool_free(&pool, b);
    forge_pool_destroy(&pool);
}

static void test_free_null_noop(void) {
    TEST_BEGIN("pool/free_null_noop");
    forge_pool_t pool;
    forge_pool_init(&pool, 64, 4);
    forge_pool_free(&pool, NULL);
    ASSERT_EQ(forge_pool_stats(&pool).in_use, 0u);
    forge_pool_destroy(&pool);
}

void test_pool_run_all(void) {
    puts("=== Pool ===");
    test_alloc_free();
    test_exhaustion_returns_null();
    test_peak_tracking();
    test_slot_reuse();
    test_free_null_noop();
}
