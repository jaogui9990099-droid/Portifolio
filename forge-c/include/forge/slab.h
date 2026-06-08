#ifndef FORGE_SLAB_H
#define FORGE_SLAB_H

#include "pool.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FORGE_SLAB_MAX_CLASSES 32

typedef struct {
    forge_pool_t pools[FORGE_SLAB_MAX_CLASSES];
    size_t       sizes[FORGE_SLAB_MAX_CLASSES];
    int          count;
} forge_slab_t;

typedef struct {
    int                class_index;
    size_t             class_size;
    forge_pool_stats_t pool;
} forge_slab_class_stats_t;

typedef struct {
    int                      class_count;
    size_t                   total_capacity;
    size_t                   total_in_use;
    forge_slab_class_stats_t classes[FORGE_SLAB_MAX_CLASSES];
} forge_slab_stats_t;

int                forge_slab_init(forge_slab_t* slab);
int                forge_slab_add_class(forge_slab_t* slab, size_t object_size, size_t capacity);
void*              forge_slab_alloc(forge_slab_t* slab, size_t size);
void               forge_slab_free(forge_slab_t* slab, void* ptr, size_t size);
forge_slab_stats_t forge_slab_stats(const forge_slab_t* slab);
void               forge_slab_destroy(forge_slab_t* slab);

#ifdef __cplusplus
}
#endif

#endif
