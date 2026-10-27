#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *pool_alloc(size_t size);
void pool_free(void *p, size_t size);

#ifdef __cplusplus
}
#endif
