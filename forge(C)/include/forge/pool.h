#ifndef FORGE_POOL_H
#define FORGE_POOL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// fixed-size object pool allocator
//
// all objects must be the same size. alloc and free are both O(1)
// with zero fragmentation.
//
// uses an intrusive free-list -- the next pointer is written directly
// into each free slot, so object size must be >= sizeof(void*).
// the pool enforces this at init time.

typedef struct {
    void*   head;           // free-list head
    void*   base;           // start of the backing buffer
    size_t  object_size;    // actual slot stride after alignment
    size_t  capacity;       // total number of slots
    size_t  in_use;
    size_t  peak;           // max concurrent in_use seen
    size_t  total_allocs;   // lifetime count
} forge_pool_t;

typedef struct {
    size_t capacity;
    size_t in_use;
    size_t available;
    size_t peak;
    size_t total_allocs;
    double utilization;     // in_use / capacity
} forge_pool_stats_t;

// init a pool for objects of @object_size bytes with @capacity pre-allocated slots.
// returns 0 on success, -1 if object_size < sizeof(void*) or allocation fails
int  forge_pool_init(forge_pool_t* pool, size_t object_size, size_t capacity);

// alloc one slot. returns NULL if the pool is exhausted
void* forge_pool_alloc(forge_pool_t* pool);

// return @ptr back to the pool -- ptr must come from this pool,
// double-free or foreign pointers are UB
void forge_pool_free(forge_pool_t* pool, void* ptr);

forge_pool_stats_t forge_pool_stats(const forge_pool_t* pool);

// free backing memory and reset to uninitialised state
void forge_pool_destroy(forge_pool_t* pool);

#ifdef __cplusplus
}
#endif

#endif /* FORGE_POOL_H */
