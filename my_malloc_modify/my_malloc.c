#include "my_malloc.h"
#include <assert.h>

void *removeNode(freeNode *node){
  if(node->prev == NULL){
    head = node->next;
  }
  else{
    node->prev->next = node->next;
  }
  if(node->next == NULL){
    tail = node->prev;
  }
  else{
    node->next->prev = node->prev;
  }
  node->next = NULL;
  node->prev = NULL;
  void *addr = (void *)node + metasize;
  return addr;
}

void *newNode(freeNode *node, freeNode *ptr, size_t diff){
  //generate a new node 
  if(node->next == NULL){
    tail = ptr;
  }
  else{
    node->next->prev = ptr; 
  }
  if(node->prev == NULL){
    head = ptr;
  }
  else{
    node->prev->next = ptr;
  }
  ptr->size = diff - metasize;
  ptr->next = node->next;
  ptr->prev = node->prev;
  node->next = NULL;
  node->prev = NULL;
  void *addr = (void *)node + metasize;
  return addr;
}

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
      //remove this node from freelist
      freeSz += (*cur)->size + metasize;
      return removeNode((*cur));
    }
    else if((*cur)->size > size){
      //need to split
      size_t diff = (*cur)->size - size;
      //if the remaining size cannot hold the metadata
      //just remove the node
      if(diff <= metasize){
	freeSz += (*cur)->size + metasize;
	return removeNode((*cur));
      }
      //split the mem
      else{
	(*cur)->size = size;
	//generate a new node
	freeNode *newptr =(freeNode *) ((void *)(*cur) + total);
	freeSz += (*cur)->size + metasize;
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
  freeSz += total;
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
      freeSz += (*cur)->size + metasize;
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
    freeSz += total;
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
      freeSz += min->size + metasize;
      return removeNode(min);
    }
    else{
      min->size = size;
      freeNode *newptr = (freeNode *)((void *)min + total);
      freeSz += min->size + metasize;
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
  freeSz -= free_ptr->size + metasize;
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
  return entireHeap - freeSz;
}


