#include "entity/Pool.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
static SRWLOCK poolLock = SRWLOCK_INIT;
#define POOL_LOCK()   AcquireSRWLockExclusive(&poolLock)
#define POOL_UNLOCK() ReleaseSRWLockExclusive(&poolLock)
#else
#include <pthread.h>
static pthread_mutex_t poolLock = PTHREAD_MUTEX_INITIALIZER;
#define POOL_LOCK()   pthread_mutex_lock(&poolLock)
#define POOL_UNLOCK() pthread_mutex_unlock(&poolLock)
#endif

#define GRAIN_SHIFT 4
#define CLASSES 32
#define SLAB (64 * 1024)

typedef struct Node {
  struct Node *next;
} Node;

typedef struct Slab {
  struct Slab *next;
} Slab;

static Node *freeList[CLASSES];
static Slab *slabs;

static size_t classOf(size_t size) {
  if (size == 0)
    size = 1;
  return (size + (1 << GRAIN_SHIFT) - 1) >> GRAIN_SHIFT;
}

static void refill(size_t c) {
  size_t size = c << GRAIN_SHIFT;
  Slab *s = (Slab *)malloc(sizeof(Slab) + SLAB);
  if (!s)
    abort();
  s->next = slabs;
  slabs = s;
  char *slab = (char *)(s + 1);
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
    if (!p)
      abort();
    return p;
  }
  POOL_LOCK();
  if (!freeList[c])
    refill(c);
  Node *n = freeList[c];
  freeList[c] = n->next;
  POOL_UNLOCK();
  return n;
}

void pool_free(void *p, size_t size) {
  if (!p)
    return;
  size_t c = classOf(size);
  if (c >= CLASSES) {
    free(p);
    return;
  }
  POOL_LOCK();
  Node *n = (Node *)p;
  n->next = freeList[c];
  freeList[c] = n;
  POOL_UNLOCK();
}

void pool_shutdown(void) {
  POOL_LOCK();
  while (slabs) {
    Slab *s = slabs;
    slabs = slabs->next;
    free(s);
  }
  for (size_t c = 0; c < CLASSES; c++)
    freeList[c] = NULL;
  POOL_UNLOCK();
}
