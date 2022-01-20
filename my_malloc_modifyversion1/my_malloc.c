#include "my_malloc.h"
#include <assert.h>

/* To remove a node from the freelist */
//when we allocate memory(malloc), we use some memory in heap
//which means this is not free memory, so remove it from freelist
//input this node, and return the address of this malloc memory
void *removeNode(freeNode *node){
  //tell if it is the first node, need to update head
  if(node->prev == NULL){
    head = node->next;
  }
  else{
    node->prev->next = node->next;
  }
  //tell if it is the last node, need to update tail  
  if(node->next == NULL){
    tail = node->prev;
  }
  else{
    node->next->prev = node->prev;
  }
  //update this node, it is not in freelist anymore
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
void *newNode(freeNode *node, freeNode *ptr, size_t diff){
  //if the ideal region is the first node, need to update head
  //pointing at the remaining region
  if(node->prev == NULL){
    head = ptr;
  }
  else{
    node->prev->next = ptr;
  }
  //if the ideal region is the last node, need to update tail
  //pointing at the remaining region
  if(node->next == NULL){
    tail = ptr;
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

/* First fit malloc */
void *ff_malloc(size_t size){
  if(size == 0){
    fprintf(stderr, "Cannot allocate 0 mem");
    return NULL;
  }
  freeNode **cur = &head;
  size_t total = size + metasize;
  while((*cur) != NULL){
    //find a matched position to allocate mem
    if((*cur)->size == size){
      //看到这里
      mallocSz += (*cur)->size + metasize;
      //remove this node from freelist
      return removeNode((*cur));
    }
    else if((*cur)->size > size){
      //need to split
      size_t diff = (*cur)->size - size;
      //if the remaining size cannot hold the metadata
      //just remove the node
      if(diff <= metasize){
	mallocSz += (*cur)->size + metasize;
	return removeNode((*cur));
      }
      //split the mem
      else{
	(*cur)->size = size;
	//generate a new node
	freeNode *newptr =(freeNode *) ((void *)(*cur) + total);
	mallocSz += (*cur)->size + metasize;
	return newNode((*cur), newptr, diff);
      }
    }
    else{
      cur = &(*cur)->next;
    }
  }
  //no matched mem, need to expand heap
  freeNode *current = sbrk(0);
  void *break_exp = sbrk(total);
  entireHeap += total;
  mallocSz += total;
  current->size = size;
  current->prev = NULL;
  current->next = NULL;
  void *addr = (void *)current + metasize;
  return addr;
}

void *bf_malloc(size_t size){
  if(size == 0){
    fprintf(stderr, "Cannot allocate 0 mem");
    return NULL;
  }
  freeNode **cur = &head;
  size_t total = size + metasize;
  freeNode *min = NULL;
  while((*cur) != NULL){
    //find a matched position to allocate mem
    if((*cur)->size == size){
      mallocSz += (*cur)->size + metasize;
      return removeNode((*cur));
    }
    else if((*cur)->size > size){
      if(min == NULL || (*cur)->size < min->size){
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
  //no matched mem, need to expand heap
  if(min == NULL){
    freeNode *current = sbrk(0);
    void *break_exp = sbrk(total);
    entireHeap += total;
    mallocSz += total;
    current->size = size;
    current->prev = NULL;
    current->next = NULL;
    void *addr = (void *)current + metasize;
    return addr;
  }
  else{
    size_t diff = min->size - size;
    //remaining size cannot hold metadata
    if(diff <= metasize){
      mallocSz += min->size + metasize;
      return removeNode(min);
    }
    else{
      min->size = size;
      freeNode *newptr = (freeNode *)((void *)min + total);
      mallocSz += min->size + metasize;
      return newNode(min, newptr, diff);
    }
  }
}

void general_free(void *ptr){
  if(ptr == NULL){
    fprintf(stderr, "Cannot free NULL pointer");
    return;
  }
  freeNode **cur = &head;
  freeNode *free_ptr = (freeNode *)(ptr - metasize);
  assert(free_ptr->next == NULL && free_ptr->prev == NULL);
  //empty freelist
  if(head == NULL){
    head = free_ptr;
    tail = free_ptr;
  }
  else{
    //find a position to add the node into the freelist
    while((*cur) != NULL && (*cur) < free_ptr){
      cur = &(*cur)->next;
    }
    //the position is before the head
    if(cur == &head){
      free_ptr->next = head;
      free_ptr->prev = NULL;
      free_ptr->next->prev = free_ptr;
      head = free_ptr;
    }
    //the position is after the tail
    else if((*cur) == NULL){
      free_ptr->next = NULL;
      tail->next = free_ptr;
      free_ptr->prev = tail;
      tail = free_ptr;
    }
    //the position is at the medium
    else{
      free_ptr->next = (*cur);
      free_ptr->prev = free_ptr->next->prev;
      free_ptr->prev->next = free_ptr;
      free_ptr->next->prev = free_ptr;
    }
  }
  mallocSz -= free_ptr->size + metasize;
  //merge if needed
  if(free_ptr == NULL){
    return;
  }
  else{
    //tell if it's needed to merge or not
    //merge with the later one
    if(free_ptr->next != NULL){
      if((void*)free_ptr->next == (void *)free_ptr + metasize + free_ptr->size){
      free_ptr->size = free_ptr->size + free_ptr->next->size + metasize;
      freeNode *temp = free_ptr->next;
      free_ptr->next = free_ptr->next->next;
      //merged node becomes tail
      if(free_ptr->next == NULL){
	tail = free_ptr;
      }
      else{
	free_ptr->next->prev = free_ptr;
      }
      temp->next = NULL;
      temp->prev = NULL;
    }
    }

    //merge with the former one
  if(free_ptr->prev != NULL){
    if ((void*)free_ptr == (void*)free_ptr->prev + free_ptr->prev->size + metasize){
      free_ptr->prev->size = free_ptr->size + free_ptr->prev->size + metasize;
      freeNode *temp = free_ptr;
      free_ptr->prev->next = free_ptr->next;
      if(free_ptr->prev->next == NULL){
	tail = free_ptr->prev;
      }
      else{
	free_ptr->prev->next->prev = free_ptr->prev;
      }
      temp->next = NULL;
      temp->prev = NULL;
    }
}
}
 }

void ff_free(void *ptr){
  general_free(ptr);
}

void bf_free(void *ptr){
  general_free(ptr);
}

unsigned long get_data_segment_size(){
  return entireHeap;
}

unsigned long get_data_segment_free_space_size(){
  freeNode **cur = &head;
  while((*cur) != NULL){
    freeSz += (*cur)->size + metasize;
    cur = &(*cur)->next;
  }
  return freeSz;
  //unsigned long freeSz;
  //freeSz = entireHeap - mallocSz;
  //return freeSz;
}


