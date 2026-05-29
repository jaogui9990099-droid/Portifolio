#ifndef FORGE_SLAB_H
#define FORGE_SLAB_H

#include "pool.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// multi-class slab allocator
//
// owns an ordered set of forge_pool_t instances, one per size class.
// alloc dispatches to the tightest-fitting class via binary search -- O(log k)
// where k is the number of registered classes (tipically < 16).
//
// built-in classes: 16, 32, 64, 128, 256, 512, 1024, 2048 bytes.
// add more with forge_slab_add_class() before the first alloc.

#define FORGE_SLAB_MAX_CLASSES 32

typedef struct {
    forge_pool_t pools[FORGE_SLAB_MAX_CLASSES];
    size_t       sizes[FORGE_SLAB_MAX_CLASSES];
    int          count;
} forge_slab_t;

typedef struct {
    int    class_index;
    size_t class_size;
    forge_pool_stats_t pool;
} forge_slab_class_stats_t;

typedef struct {
    int                     class_count;
    size_t                  total_capacity;
    size_t                  total_in_use;
    forge_slab_class_stats_t classes[FORGE_SLAB_MAX_CLASSES];
} forge_slab_stats_t;

// init the slab and register the built-in size classes.
// returns 0 on success, -1 on failure
int  forge_slab_init(forge_slab_t* slab);

// register an additional size class -- call before the first alloc.
// returns 0 on success, -1 if the class table is full or allocation fails
int  forge_slab_add_class(forge_slab_t* slab, size_t object_size, size_t capacity);

// alloc a block for an object of @size bytes.
// returns NULL if size exceeds all classes, or if the matching pool is full
void* forge_slab_alloc(forge_slab_t* slab, size_t size);

// return @ptr (allocated for @size bytes) back to the slab
void forge_slab_free(forge_slab_t* slab, void* ptr, size_t size);

forge_slab_stats_t forge_slab_stats(const forge_slab_t* slab);

void forge_slab_destroy(forge_slab_t* slab);

#ifdef __cplusplus
}
#endif

#endif /* FORGE_SLAB_H */
