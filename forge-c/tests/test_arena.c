#include "unity.h"
#include "forge/arena.h"
#include <string.h>

void test_arena_run_all(void);

static void test_basic_alloc(void) {
    TEST_BEGIN("arena/basic_alloc");
    forge_arena_t a;
    ASSERT_EQ(forge_arena_init(&a, 4096), 0);
    void* p1 = forge_arena_alloc(&a, 128);
    void* p2 = forge_arena_alloc(&a, 256);
    ASSERT_NOTNULL(p1);
    ASSERT_NOTNULL(p2);
    ASSERT_NE(p1, p2);
    forge_arena_destroy(&a);
}

static void test_alignment(void) {
    TEST_BEGIN("arena/alignment");
    forge_arena_t a;
    forge_arena_init(&a, 4096);
    for (int i = 1; i < 64; ++i) {
        void* p = forge_arena_alloc(&a, (size_t)i);
        ASSERT_NOTNULL(p);
        ASSERT_EQ(((uintptr_t)p) % _Alignof(max_align_t), 0u);
    }
    forge_arena_destroy(&a);
}

static void test_reset_reuses_memory(void) {
    TEST_BEGIN("arena/reset_reuses_memory");
    forge_arena_t a;
    forge_arena_init(&a, 1024);
    void* p1 = forge_arena_alloc(&a, 256);
    ASSERT_NOTNULL(p1);
    forge_arena_reset(&a);
    void* p2 = forge_arena_alloc(&a, 256);
    ASSERT_NOTNULL(p2);
    ASSERT_EQ(p1, p2);
    forge_arena_destroy(&a);
}

static void test_overflow_chaining(void) {
    TEST_BEGIN("arena/overflow_chaining");
    forge_arena_t a;
    forge_arena_init(&a, 128);
    int success = 1;
    for (int i = 0; i < 32; ++i) {
        if (!forge_arena_alloc(&a, 64)) { success = 0; break; }
    }
    ASSERT_TRUE(success);
    forge_arena_stats_t s = forge_arena_stats(&a);
    ASSERT_TRUE(s.overflow_count > 0);
    forge_arena_destroy(&a);
}

static void test_calloc_zeroes(void) {
    TEST_BEGIN("arena/calloc_zeroes");
    forge_arena_t a;
    forge_arena_init(&a, 4096);
    int* arr = forge_arena_calloc(&a, 64, sizeof(int));
    ASSERT_NOTNULL(arr);
    int all_zero = 1;
    for (int i = 0; i < 64; ++i) if (arr[i] != 0) { all_zero = 0; break; }
    ASSERT_TRUE(all_zero);
    forge_arena_destroy(&a);
}

static void test_zero_alloc_returns_null(void) {
    TEST_BEGIN("arena/zero_alloc_returns_null");
    forge_arena_t a;
    forge_arena_init(&a, 1024);
    ASSERT_NULL(forge_arena_alloc(&a, 0));
    forge_arena_destroy(&a);
}

void test_arena_run_all(void) {
    puts("=== Arena ===");
    test_basic_alloc();
    test_alignment();
    test_reset_reuses_memory();
    test_overflow_chaining();
    test_calloc_zeroes();
    test_zero_alloc_returns_null();
}
