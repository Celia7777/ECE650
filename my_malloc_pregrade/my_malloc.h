#include <stdio.h>
#include <unistd.h>

#define metasize sizeof(freeNode)

//Data structure that represents a list of free memory regions
struct freeNode_t{
  size_t size;
  struct freeNode_t * next;
  struct freeNode_t * prev;
};
typedef struct freeNode_t freeNode;

freeNode *head = NULL;
freeNode *tail = NULL;

//helper functions
void *removeNode(freeNode *node);
void *newNode(freeNode *node, freeNode *ptr, size_t diff);

//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//Performance study
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes

unsigned long entireHeap = 0;
unsigned long freeSz = 0;

