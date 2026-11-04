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

#define SLAB (64 * 1024)
#ifdef __SANITIZE_ADDRESS__
#define DIRECT 1
#else
#define DIRECT 0
#endif

typedef struct Node {
  struct Node *next;
} Node;

typedef struct Slab {
  struct Slab *next;
} Slab;

static Node *freeList[POOL_CLASSES];
static Slab *slabs;

static size_t classOf(size_t size) {
  if (size == 0)
    size = 1;
  return (size + (1 << POOL_GRAIN_SHIFT) - 1) >> POOL_GRAIN_SHIFT;
}

static void refill(size_t c) {
  size_t size = c << POOL_GRAIN_SHIFT;
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
  if (c >= POOL_CLASSES || DIRECT) {
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
  if (c >= POOL_CLASSES || DIRECT) {
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
  for (size_t c = 0; c < POOL_CLASSES; c++)
    freeList[c] = NULL;
  POOL_UNLOCK();
}
