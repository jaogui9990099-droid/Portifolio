#include "forge/arena.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static inline size_t align_up(size_t n, size_t align) {
    assert(align > 0 && (align & (align - 1)) == 0);
    return (n + align - 1) & ~(align - 1);
}

int forge_arena_init(forge_arena_t* arena, size_t capacity) {
    assert(arena && capacity > 0);
    unsigned char* base = (unsigned char*)malloc(capacity);
    if (!base) return -1;
    arena->base      = base;
    arena->offset    = 0;
    arena->capacity  = capacity;
    arena->next      = NULL;
    arena->owns_base = 1;
    return 0;
}

void forge_arena_init_from(forge_arena_t* arena, void* buf, size_t capacity) {
    assert(arena && buf && capacity > 0);
    arena->base      = (unsigned char*)buf;
    arena->offset    = 0;
    arena->capacity  = capacity;
    arena->next      = NULL;
    arena->owns_base = 0;
}

void* forge_arena_alloc_aligned(forge_arena_t* arena, size_t size, size_t align) {
    assert(arena);
    assert(align > 0 && (align & (align - 1)) == 0);
    if (size == 0) return NULL;

    size_t aligned_offset = align_up(arena->offset, align);
    size_t end            = aligned_offset + size;

    if (end <= arena->capacity) {
        void* ptr     = arena->base + aligned_offset;
        arena->offset = end;
        return ptr;
    }

    if (!arena->next) {
        size_t overflow_cap = arena->capacity > size ? arena->capacity : size * 2;
        arena->next = (forge_arena_t*)malloc(sizeof(forge_arena_t));
        if (!arena->next) return NULL;
        if (forge_arena_init(arena->next, overflow_cap) != 0) {
            free(arena->next);
            arena->next = NULL;
            return NULL;
        }
    }

    return forge_arena_alloc_aligned(arena->next, size, align);
}

void* forge_arena_alloc(forge_arena_t* arena, size_t size) {
    return forge_arena_alloc_aligned(arena, size, FORGE_ARENA_DEFAULT_ALIGNMENT);
}

void* forge_arena_calloc(forge_arena_t* arena, size_t count, size_t size) {
    size_t total = count * size;
    void*  ptr   = forge_arena_alloc(arena, total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void forge_arena_reset(forge_arena_t* arena) {
    assert(arena);
    forge_arena_t* overflow = arena->next;
    while (overflow) {
        forge_arena_t* next = overflow->next;
        if (overflow->owns_base) free(overflow->base);
        free(overflow);
        overflow = next;
    }
    arena->offset = 0;
    arena->next   = NULL;
}

void forge_arena_destroy(forge_arena_t* arena) {
    assert(arena);
    forge_arena_reset(arena);
    if (arena->owns_base) {
        free(arena->base);
        arena->base = NULL;
    }
}

forge_arena_stats_t forge_arena_stats(const forge_arena_t* arena) {
    forge_arena_stats_t s = {0, 0, 0, 0};
    const forge_arena_t* node = arena;
    size_t peak_acc = 0;
    while (node) {
        s.used     += node->offset;
        s.capacity += node->capacity;
        if (node->offset > peak_acc) peak_acc = node->offset;
        if (node->next) s.overflow_count++;
        node = node->next;
    }
    s.peak = peak_acc;
    return s;
}
