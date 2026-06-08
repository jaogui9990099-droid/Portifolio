#include "forge/pool.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MIN_OBJECT_SIZE (sizeof(void*))

static size_t slot_stride(size_t object_size) {
    size_t align = _Alignof(max_align_t);
    return (object_size + align - 1) & ~(align - 1);
}

int forge_pool_init(forge_pool_t* pool, size_t object_size, size_t capacity) {
    assert(pool && capacity > 0);
    if (object_size < MIN_OBJECT_SIZE) object_size = MIN_OBJECT_SIZE;

    size_t stride = slot_stride(object_size);
    void*  base   = calloc(capacity, stride);
    if (!base) return -1;

    unsigned char* p = (unsigned char*)base;
    for (size_t i = 0; i < capacity - 1; ++i)
        *(void**)(p + i * stride) = p + (i + 1) * stride;
    *(void**)(p + (capacity - 1) * stride) = NULL;

    pool->head         = base;
    pool->base         = base;
    pool->object_size  = stride;
    pool->capacity     = capacity;
    pool->in_use       = 0;
    pool->peak         = 0;
    pool->total_allocs = 0;
    return 0;
}

void* forge_pool_alloc(forge_pool_t* pool) {
    assert(pool);
    if (!pool->head) return NULL;
    void* ptr  = pool->head;
    pool->head = *(void**)ptr;
    pool->in_use++;
    pool->total_allocs++;
    if (pool->in_use > pool->peak) pool->peak = pool->in_use;
#if defined(FORGE_DEBUG_FILL) && FORGE_DEBUG_FILL
    memset(ptr, 0xAB, pool->object_size);
#endif
    return ptr;
}

void forge_pool_free(forge_pool_t* pool, void* ptr) {
    assert(pool);
    if (!ptr) return;
#if defined(FORGE_DEBUG_FILL) && FORGE_DEBUG_FILL
    memset(ptr, 0xDF, pool->object_size);
#endif
    *(void**)ptr = pool->head;
    pool->head   = ptr;
    pool->in_use--;
}

forge_pool_stats_t forge_pool_stats(const forge_pool_t* pool) {
    assert(pool);
    forge_pool_stats_t s = {
        .capacity     = pool->capacity,
        .in_use       = pool->in_use,
        .available    = pool->capacity - pool->in_use,
        .peak         = pool->peak,
        .total_allocs = pool->total_allocs,
        .utilization  = pool->capacity > 0
                        ? (double)pool->in_use / (double)pool->capacity
                        : 0.0,
    };
    return s;
}

void forge_pool_destroy(forge_pool_t* pool) {
    assert(pool);
    free(pool->base);
    memset(pool, 0, sizeof(*pool));
}
