#ifndef FORGE_POOL_H
#define FORGE_POOL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void*   head;
    void*   base;
    size_t  object_size;
    size_t  capacity;
    size_t  in_use;
    size_t  peak;
    size_t  total_allocs;
} forge_pool_t;

typedef struct {
    size_t capacity;
    size_t in_use;
    size_t available;
    size_t peak;
    size_t total_allocs;
    double utilization;
} forge_pool_stats_t;

int                forge_pool_init(forge_pool_t* pool, size_t object_size, size_t capacity);
void*              forge_pool_alloc(forge_pool_t* pool);
void               forge_pool_free(forge_pool_t* pool, void* ptr);
forge_pool_stats_t forge_pool_stats(const forge_pool_t* pool);
void               forge_pool_destroy(forge_pool_t* pool);

#ifdef __cplusplus
}
#endif

#endif
