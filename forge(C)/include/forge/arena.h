#ifndef FORGE_ARENA_H
#define FORGE_ARENA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// bump-pointer arena allocator
//
// memory is comited from a single contiguous backing store.
// individual allocations cant be freed -- everything goes at once via
// forge_arena_reset() or forge_arena_destroy().
// if the primary node runs out, overflow arenas are chained automaticaly.

#define FORGE_ARENA_DEFAULT_ALIGNMENT (_Alignof(max_align_t))

typedef struct forge_arena forge_arena_t;

struct forge_arena {
    unsigned char*  base;
    size_t          offset;
    size_t          capacity;
    forge_arena_t*  next;       // overflow chain, usually NULL
    int             owns_base;  // 0 if the buffer was provided by the caller
};

typedef struct {
    size_t used;
    size_t peak;
    size_t capacity;
    size_t overflow_count;
} forge_arena_stats_t;

// init an arena backed by a newly allocated block of @capacity bytes.
// returns 0 on success, -1 if malloc fails.
int  forge_arena_init(forge_arena_t* arena, size_t capacity);

// init over an existing buffer -- caller owns the buffers lifetime
void forge_arena_init_from(forge_arena_t* arena, void* buf, size_t capacity);

// alloc @size bytes with default alignment.
// returns NULL only if size == 0 or a new backing block cant be obtained
void* forge_arena_alloc(forge_arena_t* arena, size_t size);

// alloc @size bytes aligned to @align (must be a power of two)
void* forge_arena_alloc_aligned(forge_arena_t* arena, size_t size, size_t align);

// alloc and zero-init @count elements of @size bytes each
void* forge_arena_calloc(forge_arena_t* arena, size_t count, size_t size);

// rewind the bump pointer to zero, free overflow arenas, keep primary buffer
void forge_arena_reset(forge_arena_t* arena);

// free everything, including the primary buffer if internally allocated
void forge_arena_destroy(forge_arena_t* arena);

// return usage stats accumulated across the whole chain
forge_arena_stats_t forge_arena_stats(const forge_arena_t* arena);

#ifdef __cplusplus
}
#endif

#endif /* FORGE_ARENA_H */
