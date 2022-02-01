#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


//Data structure that represents a list of free memory regions
struct freeNode_t{
  size_t size;
  struct freeNode_t * next;
  struct freeNode_t * prev;
};
typedef struct freeNode_t freeNode;

#define metasize sizeof(freeNode)

// for 2-versions safe-threads
// for lock version
freeNode *head = NULL;
freeNode *tail = NULL;
// for nolock version
// initialize a mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
__thread freeNode *head_nolock = NULL;
__thread freeNode *tail_nolock = NULL;

//helper functions
void *removeNode(freeNode *node, freeNode **head, freeNode **tail);
void *newNode(freeNode *node, freeNode *ptr, size_t diff, freeNode **head, freeNode **tail);
void* expandHeap_lock(size_t size);
void* expandHeap_nolock(size_t size);

//Best Fit malloc/free
void *bf_malloc(size_t size, freeNode **head, freeNode **tail, int version);

// free
void general_free(void *ptr, freeNode **head, freeNode **tail);

//Thread Safe malloc/free: locking version
void * ts_malloc_lock(size_t size);
void ts_free_lock(void * ptr);

//Thread Safe malloc/free: non-locking version
void * ts_malloc_nolock(size_t size);
void ts_free_nolock(void * ptr);

