#ifndef FORGE_ARENA_H
#define FORGE_ARENA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FORGE_ARENA_DEFAULT_ALIGNMENT (_Alignof(max_align_t))

typedef struct forge_arena forge_arena_t;

struct forge_arena {
    unsigned char*  base;
    size_t          offset;
    size_t          capacity;
    forge_arena_t*  next;
    int             owns_base;
};

typedef struct {
    size_t used;
    size_t peak;
    size_t capacity;
    size_t overflow_count;
} forge_arena_stats_t;

int   forge_arena_init(forge_arena_t* arena, size_t capacity);
void  forge_arena_init_from(forge_arena_t* arena, void* buf, size_t capacity);
void* forge_arena_alloc(forge_arena_t* arena, size_t size);
void* forge_arena_alloc_aligned(forge_arena_t* arena, size_t size, size_t align);
void* forge_arena_calloc(forge_arena_t* arena, size_t count, size_t size);
void  forge_arena_reset(forge_arena_t* arena);
void  forge_arena_destroy(forge_arena_t* arena);
forge_arena_stats_t forge_arena_stats(const forge_arena_t* arena);

#ifdef __cplusplus
}
#endif

#endif
