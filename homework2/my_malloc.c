#include "my_malloc.h"
#include <assert.h>

/* To remove a node from the freelist */
//when we allocate memory(malloc), we use some memory in heap
//which means this is not free memory, so remove it from freelist
//input this node, and return the address of this malloc memory
void *removeNode(freeNode *node, freeNode **head, freeNode **tail){
  //tell if it is the first node, need to update head
  if(node->prev == NULL){
    *head = node->next;
  }
  else{
    node->prev->next = node->next;
  }
  //tell if it is the last node, need to update tail  
  if(node->next == NULL){
    *tail = node->prev;
  }
  else{
    node->next->prev = node->prev;
  }
  //update this node, it is not in freelist anymore
  //node->size = size;
  node->next = NULL;
  node->prev = NULL;
  void *addr = (void *)node + metasize;
  return addr;
}

/* To add a node into freelist */
//when the ideal free region is larger than requested size
//it may need to split this region, so we add the remaining
//region to the freelist
//input a pointer to the start of this ideal region, a pointer
//to the start of the remaining region, and the size of this
//remaining region
void *newNode(freeNode *node, freeNode *ptr, size_t diff, freeNode **head, freeNode **tail){
  //if the ideal region is the first node, need to update head
  //pointing at the remaining region
  if(node->prev == NULL){
    *head = ptr;
  }
  else{
    node->prev->next = ptr;
  }
  //if the ideal region is the last node, need to update tail
  //pointing at the remaining region
  if(node->next == NULL){
    *tail = ptr;
  }
  else{
    node->next->prev = ptr; 
  }
  //update the remaining region as a new node in the freelist
  ptr->size = diff - metasize;
  ptr->next = node->next;
  ptr->prev = node->prev;
  node->next = NULL;
  node->prev = NULL;
  void *addr = (void *)node + metasize;
  return addr;
}

/* To expand heap for lock version*/
//if there is no free space that fits an allocation request,
//then sbrk() should be used to create that space
void* expandHeap_lock(size_t size){
  size_t total = size + metasize;
  freeNode *current = NULL;
  current = sbrk(total);
  current->size = size;
  current->prev = NULL;
  current->next = NULL;
  void *addr = (void *)current + metasize;
  return addr;
}

/* To expand heap for nolock version */
//if there is no free space that fits an allocation request,
//then sbrk() should be used to create that space
void* expandHeap_nolock(size_t size){
  size_t total = size + metasize;
  freeNode *current = NULL;
  pthread_mutex_lock(&lock);
  current = sbrk(total);
  pthread_mutex_unlock(&lock);
  current->size = size;
  current->prev = NULL;
  current->next = NULL;
  void *addr = (void *)current + metasize;
  return addr;
}


//version=0: lock version
//version=1: nolock version
void *bf_malloc(size_t size, freeNode **head, freeNode **tail, int version){
  if(size == 0){
    fprintf(stderr, "Cannot allocate 0 mem");
    return NULL;
  }
  freeNode **cur = head;
  size_t total = size + metasize;
  freeNode *min = NULL;
  while((*cur) != NULL){
    //find a matched position to allocate mem
    if((*cur)->size == size){
      //remove this node from freelist
      return removeNode((*cur), head, tail);
    }
    //find a best matched position
    else if((*cur)->size > size){
      if(min == NULL || (*cur)->size < min->size){
    //find a posible region, use min to record
        min = (*cur);
        cur = &(*cur)->next;
      }
      else{
        cur = &(*cur)->next;
      }
    }
    else{
      cur = &(*cur)->next;
    }
  }
  //no matched region, need to expand heap
  if(min == NULL){
    if(version == 0){
      return expandHeap_lock(size);
    }
    if(version == 1){
      return expandHeap_nolock(size);
    }
  }
  //find one best matched region, may need to split
  else{
    size_t diff = min->size - size;
    //remaining size cannot hold metadata
    //just remove the node, no need to split
    if(diff <= metasize){
      return removeNode(min, head, tail);
    }
    //split the mem 
    else{
      min->size = size;
      freeNode *newptr = (freeNode *)((void *)min + total);
      //generate a new node(remaining region) into the freelist
      return newNode(min, newptr, diff, head, tail);
    }
  }
}

/* Helper function to free memory */
//for both first fit and best fit
void general_free(void *ptr, freeNode **head, freeNode **tail){
  if(ptr == NULL){
    fprintf(stderr, "Cannot free NULL pointer");
    return;
  }
  freeNode **cur = head;
  //get the start of the free region
  freeNode *free_ptr = (freeNode *)(ptr - metasize);
  //assert(free_ptr->next == NULL && free_ptr->prev == NULL);
  //empty freelist, just add this region to freelist
  if(*head == NULL){
    *head = free_ptr;
    *tail = free_ptr;
  }
  else{
    //find a position to add the node into the freelist
    //current position is behind the free region
    while((*cur) != NULL && (*cur) < free_ptr){
      cur = &(*cur)->next;
    }
    //the position is before the head
    if(cur == head){
      free_ptr->next = *head;
      free_ptr->prev = NULL;
      free_ptr->next->prev = free_ptr;
      *head = free_ptr;
    }
    //the position is after the tail
    else if((*cur) == NULL){
      free_ptr->next = NULL;
      (*tail)->next = free_ptr;
      free_ptr->prev = *tail;
      *tail = free_ptr;
    }
    //the position is at the middle
    else{
      free_ptr->next = (*cur);
      free_ptr->prev = free_ptr->next->prev;
      free_ptr->prev->next = free_ptr;
      free_ptr->next->prev = free_ptr;
    }
  }
  //handle merging if needed
  //tell if it's needed to merge or not
  //merge with the later one, make sure the later one is not NULL
  if(free_ptr->next != NULL){
    //find the adjacent nodes
    if((void*)free_ptr->next == (void *)free_ptr + metasize + free_ptr->size){
      free_ptr->size = free_ptr->size + free_ptr->next->size + metasize;
      freeNode *temp = free_ptr->next;
      free_ptr->next = free_ptr->next->next;
      //merged node becomes tail
      if(free_ptr->next == NULL){
	      *tail = free_ptr;
      }
      else{
	      free_ptr->next->prev = free_ptr;
      }
      temp->next = NULL;
      temp->prev = NULL;
    }
  }
  //merge with the former one, make sure we have the former one
  if(free_ptr->prev != NULL){
    //find the adjacent nodes before
    if ((void*)free_ptr == (void*)free_ptr->prev + free_ptr->prev->size + metasize){
      free_ptr->prev->size = free_ptr->size + free_ptr->prev->size + metasize;
      freeNode *temp = free_ptr;
      free_ptr->prev->next = free_ptr->next;
      //current node is the tail, merged with the former one
      //the merged new node becomes the tail
      if(free_ptr->prev->next == NULL){
	      *tail = free_ptr->prev;
      }
      else{
	      free_ptr->prev->next->prev = free_ptr->prev;
      }
      temp->next = NULL;
      temp->prev = NULL;
    }
}
}


//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    int version = 0;
    void * addr = bf_malloc(size, &head, &tail, version);
    pthread_mutex_unlock(&lock);
    return addr;
}
void ts_free_lock(void *ptr){
    pthread_mutex_lock(&lock);
    general_free(ptr, &head, &tail);
    pthread_mutex_unlock(&lock);
}

//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size){
    int version = 1;
    void * addr = bf_malloc(size, &head_nolock, &tail_nolock, version);
    return addr;
}
void ts_free_nolock(void *ptr){
    general_free(ptr, &head_nolock, &tail_nolock);
}