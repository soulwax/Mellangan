#pragma once

#include <stddef.h>

#define POOL_GRAIN_SHIFT 4
#define POOL_CLASSES 96
#define POOL_MAX ((POOL_CLASSES - 1) << POOL_GRAIN_SHIFT)

#ifdef __cplusplus
extern "C" {
#endif

void *pool_alloc(size_t size);
void pool_free(void *p, size_t size);
void pool_shutdown(void);

#ifdef __cplusplus
}
#endif
