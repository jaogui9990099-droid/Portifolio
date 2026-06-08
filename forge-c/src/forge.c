#include "forge/forge.h"
#include <stdio.h>

const char* forge_version_string(void) {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%d.%d.%d",
             FORGE_VERSION_MAJOR, FORGE_VERSION_MINOR, FORGE_VERSION_PATCH);
    return buf;
}
