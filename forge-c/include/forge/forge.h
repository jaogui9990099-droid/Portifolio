#ifndef FORGE_H
#define FORGE_H

#include "arena.h"
#include "pool.h"
#include "slab.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FORGE_VERSION_MAJOR 1
#define FORGE_VERSION_MINOR 0
#define FORGE_VERSION_PATCH 0
#define FORGE_VERSION \
    ((FORGE_VERSION_MAJOR << 16) | (FORGE_VERSION_MINOR << 8) | FORGE_VERSION_PATCH)

const char* forge_version_string(void);

#ifdef __cplusplus
}
#endif

#endif
