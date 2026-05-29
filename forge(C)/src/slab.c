#include "forge/slab.h"

#include <assert.h>
#include <string.h>

static const size_t k_builtin_sizes[]    = { 16, 32, 64, 128, 256, 512, 1024, 2048 };
static const size_t k_builtin_capacity[] = { 4096, 4096, 2048, 1024, 512, 256, 128, 64 };
static const int    k_builtin_count =
    (int)(sizeof(k_builtin_sizes) / sizeof(k_builtin_sizes[0]));

// returns index of the smallest class >= size, or -1 if none found
static int find_class(const forge_slab_t* slab, size_t size) {
    int lo = 0, hi = slab->count - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (slab->sizes[mid] == size) return mid;
        if (slab->sizes[mid] < size) lo = mid + 1;
        else                         hi = mid - 1;
    }
    return lo < slab->count ? lo : -1;
}

static int insert_sorted(forge_slab_t* slab, size_t object_size, size_t capacity) {
    if (slab->count >= FORGE_SLAB_MAX_CLASSES) return -1;

    int pos = slab->count;
    for (int i = 0; i < slab->count; ++i) {
        if (slab->sizes[i] == object_size) return 0; // already registered, skip
        if (slab->sizes[i] > object_size) { pos = i; break; }
    }

    // shift everything right to make room
    for (int i = slab->count; i > pos; --i) {
        slab->sizes[i] = slab->sizes[i - 1];
        slab->pools[i] = slab->pools[i - 1];
    }

    slab->sizes[pos] = object_size;
    memset(&slab->pools[pos], 0, sizeof(forge_pool_t));
    if (forge_pool_init(&slab->pools[pos], object_size, capacity) != 0) {
        // undo the shift on failure, dont leave the table in a half state
        for (int i = pos; i < slab->count - 1; ++i) {
            slab->sizes[i] = slab->sizes[i + 1];
            slab->pools[i] = slab->pools[i + 1];
        }
        return -1;
    }

    slab->count++;
    return 0;
}

int forge_slab_init(forge_slab_t* slab) {
    assert(slab);
    memset(slab, 0, sizeof(*slab));

    for (int i = 0; i < k_builtin_count; ++i) {
        if (insert_sorted(slab, k_builtin_sizes[i], k_builtin_capacity[i]) != 0) {
            forge_slab_destroy(slab);
            return -1;
        }
    }
    return 0;
}

int forge_slab_add_class(forge_slab_t* slab, size_t object_size, size_t capacity) {
    assert(slab);
    return insert_sorted(slab, object_size, capacity);
}

void* forge_slab_alloc(forge_slab_t* slab, size_t size) {
    assert(slab);
    if (size == 0) return NULL;

    int idx = find_class(slab, size);
    if (idx < 0) return NULL;

    return forge_pool_alloc(&slab->pools[idx]);
}

void forge_slab_free(forge_slab_t* slab, void* ptr, size_t size) {
    assert(slab);
    if (!ptr || size == 0) return;

    int idx = find_class(slab, size);
    if (idx < 0) return; // size wasnt registered, just ignore it

    forge_pool_free(&slab->pools[idx], ptr);
}

forge_slab_stats_t forge_slab_stats(const forge_slab_t* slab) {
    assert(slab);
    forge_slab_stats_t s;
    memset(&s, 0, sizeof(s));
    s.class_count = slab->count;

    for (int i = 0; i < slab->count; ++i) {
        forge_pool_stats_t ps = forge_pool_stats(&slab->pools[i]);
        s.classes[i].class_index = i;
        s.classes[i].class_size  = slab->sizes[i];
        s.classes[i].pool        = ps;
        s.total_capacity += ps.capacity;
        s.total_in_use   += ps.in_use;
    }
    return s;
}

void forge_slab_destroy(forge_slab_t* slab) {
    assert(slab);
    for (int i = 0; i < slab->count; ++i)
        forge_pool_destroy(&slab->pools[i]);
    memset(slab, 0, sizeof(*slab));
}
