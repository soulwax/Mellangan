#include "entity/Pool.h"

#include <stdlib.h>

#define GRAIN_SHIFT 4
#define CLASSES 32
#define SLAB (64 * 1024)

typedef struct Node {
    struct Node *next;
} Node;

static Node *freeList[CLASSES];

static size_t classOf(size_t size) { return (size + (1 << GRAIN_SHIFT) - 1) >> GRAIN_SHIFT; }


static void refill(size_t c) {
    size_t size = c << GRAIN_SHIFT;
    char *slab = (char *)malloc(SLAB);
    if (!slab) abort();
    for (size_t i = SLAB / size; i-- > 0;) {
        Node *n = (Node *)(slab + i * size);
        n->next = freeList[c];
        freeList[c] = n;
    }
}

void *pool_alloc(size_t size) {
    size_t c = classOf(size);
    if (c >= CLASSES) {
        void *p = malloc(size);
        if (!p) abort();
        return p;
    }
    if (!freeList[c]) refill(c);
    Node *n = freeList[c];
    freeList[c] = n->next;
    return n;
}

void pool_free(void *p, size_t size) {
    size_t c = classOf(size);
    if (!p) return;
    if (c >= CLASSES) {
        free(p);
        return;
    }
    Node *n = (Node *)p;
    n->next = freeList[c];
    freeList[c] = n;
}
