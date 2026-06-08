#include "unity.h"
#include "forge/slab.h"

void test_arena_run_all(void);
void test_pool_run_all(void);
void test_slab_run_all(void);

static void test_builtin_classes(void) {
    TEST_BEGIN("slab/builtin_classes");
    forge_slab_t slab;
    ASSERT_EQ(forge_slab_init(&slab), 0);
    static const size_t sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    for (int i = 0; i < 8; ++i) {
        void* p = forge_slab_alloc(&slab, sizes[i]);
        ASSERT_NOTNULL(p);
        forge_slab_free(&slab, p, sizes[i]);
    }
    forge_slab_destroy(&slab);
}

static void test_size_rounding_up(void) {
    TEST_BEGIN("slab/size_rounding_up");
    forge_slab_t slab;
    forge_slab_init(&slab);
    void* p = forge_slab_alloc(&slab, 48);
    ASSERT_NOTNULL(p);
    forge_slab_free(&slab, p, 48);
    forge_slab_destroy(&slab);
}

static void test_oversized_returns_null(void) {
    TEST_BEGIN("slab/oversized_returns_null");
    forge_slab_t slab;
    forge_slab_init(&slab);
    ASSERT_NULL(forge_slab_alloc(&slab, 65536));
    forge_slab_destroy(&slab);
}

static void test_custom_class(void) {
    TEST_BEGIN("slab/custom_class");
    forge_slab_t slab;
    forge_slab_init(&slab);
    ASSERT_EQ(forge_slab_add_class(&slab, 4096, 16), 0);
    void* p = forge_slab_alloc(&slab, 4096);
    ASSERT_NOTNULL(p);
    forge_slab_free(&slab, p, 4096);
    forge_slab_destroy(&slab);
}

static void test_stats(void) {
    TEST_BEGIN("slab/stats");
    forge_slab_t slab;
    forge_slab_init(&slab);
    void* p = forge_slab_alloc(&slab, 64);
    ASSERT_NOTNULL(p);
    forge_slab_stats_t s = forge_slab_stats(&slab);
    ASSERT_TRUE(s.total_in_use == 1u);
    ASSERT_TRUE(s.class_count >= 8);
    forge_slab_free(&slab, p, 64);
    forge_slab_destroy(&slab);
}

void test_slab_run_all(void) {
    puts("=== Slab ===");
    test_builtin_classes();
    test_size_rounding_up();
    test_oversized_returns_null();
    test_custom_class();
    test_stats();
}

int main(void) {
    puts("forge tests\n");
    test_arena_run_all();
    test_pool_run_all();
    test_slab_run_all();
    TEST_REPORT();
    return 0;
}
