#include "my_malloc.h"
#include <assert.h>

/* void *generateNew(size_t size){ */
/*     freeNode *newOne = sbrk(0); */
/*     size_t required = size + metasize; */
/*     void *ptr = sbrk(required); */
/*     totalSBRKed += required; */
/*     allocated += required; */
/*     if(ptr==newOne-1){ */
/*         return NULL; */
/*     }else{ */
/*         newOne->size = size; */
/*         newOne->prev = NULL; */
/*         newOne->next = NULL; */
/*         return (void *)newOne + metasize; */
/*     } */
/* } */

/* void *removeFromLst(freeNode *block,size_t size){ */
/*     assert(block->size==size || block->size > size + metasize); */
/*     if(block->size==size){ */
/*         if(block->prev!=NULL){ */
/*             block->prev->next = block->next; */
/*         }else{ */
/*             head = block->next; */
/*         } */
/*         if(block->next!=NULL){ */
/*             block->next->prev = block->prev; */
/*         }else{ */
/*             tail = block->prev; */
/*         } */
/*         block->prev = NULL; */
/*         block->next = NULL; */
/*     }else{ */
/*         freeNode *redundant = (freeNode *)((void *)block + metasize + size); */
/*         redundant->size = block->size - size - metasize; */
/*         redundant->next = block->next; */
/*         redundant->prev = block->prev; */
/*         if(redundant->prev!=NULL){ */
/*             redundant->prev->next = redundant; */
/*         }else{ */
/*             head = redundant; */
/*         } */
/*         if(redundant->next!=NULL){ */
/*             redundant->next->prev = redundant; */
/*         }else{ */
/*             tail = redundant; */
/*         } */
/*         block->size = size; */
/*         block->prev = NULL; */
/*         block->next = NULL; */
/*     } */
/*     allocated += block->size+metasize; */
/*     return (void *)block + metasize; */
/* } */

/* void *ff_malloc(size_t size){ */
/*     if(size==0){ */
/*         perror("Cannot malloc 0 size memory."); */
/*         return NULL; */
/*     } */
/*     freeNode *traversal = head; */
/*     while(traversal!=NULL){ */
/*         if(traversal->size==size){ */
/*             return removeFromLst(traversal,size); */
/*         }else if(traversal->size > size){ */
/*             if(traversal->size > size + metasize){ */
/*                 return removeFromLst(traversal,size); */
/*             }else{ */
/*                 traversal = traversal->next; */
/*             } */
/*         }else{ */
/*             traversal = traversal->next; */
/*         } */
/*     } */
/*     void *ptr = generateNew(size); */
/*     if(ptr==NULL){ */
/*         perror("Malloc failed."); */
/*     } */
/*     return ptr; */
/* } */



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

/* void *newNode(freeNode *node, freeNode *ptr, size_t sz){ */
/*   //generate a new node */
/*   ptr->size = node->size - sz - metasize; */
/*   ptr->next = node->next; */
/*   ptr->prev = node->prev; */
/*   if(ptr->next == NULL){ */
/*     tail = ptr; */
/*   } */
/*   else{ */
/*     ptr->next->prev = ptr; */
/*   } */
/*   if(ptr->prev == NULL){ */
/*     head = ptr; */
/*   } */
/*   else{ */
/*     ptr->prev->next = ptr; */
/*   } */
/*   node->size = sz; */
/*   node->next = NULL; */
/*   node->prev = NULL; */
/*   void *addr = (void *)node + metasize; */
/*   return addr; */
/* } */

//我自己的
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
  //empty heap
  /* if((*cur) == NULL){ */
  /*   void *brkp = sbrk(total); */
  /*   void *addr = brkp + metasize; */
  /*   return addr; */
  /* } */
  while((*cur) != NULL){
    //find a matched position to allocate mem
    if((*cur)->size == size){
      //remove this node from freelist
      return removeNode((*cur));
    }
    else if((*cur)->size > size){
      //need to split
      //不要用减法,大于0才行
      size_t diff = (*cur)->size - size;
      //if the remaining size cannot hold the metadata
      //just remove the node
      if(diff <= metasize){
	return removeNode((*cur));
      }
      //split the mem
      else{
	(*cur)->size = size;
	//generate a new node
	freeNode *newptr =(freeNode *) ((void *)(*cur) + total);
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
  current->size = size;
  current->prev = NULL;
  current->next = NULL;
  void *addr = (void *)current + metasize;
  return addr;
}

/* void *bf_malloc(size_t size){ */
/*   freeNode **cur = &head; */
/*   size_t total = size + metasize; */
/*   //empty heap */
/*   if((*cur) == NULL){ */
/*     void *brkp = sbrk(total); */
/*     void *addr = brkp + metasize; */
/*     return addr; */
/*   } */
/*   //use a pointer to record the min mem */
/*   freeNode *min = NULL; */
/*   while((*cur) != NULL){ */
/*     //find a matched position to allocate mem */
/*     if((*cur)->size == size){ */
/*       min = (*cur); */
/*       return removeNode((*cur)); */
/*     } */
/*     else if((*cur)->size > size){ */
/*       if((*cur)->size <= min->size){ */
/* 	min = (*cur); */
/*       } */
/*       else{ */
/* 	cur = &(*cur)->next; */
/*       } */
/*     } */
/*     else{ */
/*       cur = &(*cur)->next; */
/*     } */
/*   } */
/*   //no matched mem, need to expand heap */
/*   if(min == NULL){ */
/*     void *break_exp = sbrk(total); */
/*     void *addr = (void *)break_exp + metasize; */
/*     return addr; */
/*   } */
/*   else{ */
/*     size_t diff = min->size - size; */
/*     //remaining size cannot hold metadata */
/*     if(diff <= metasize){ */
/*       return removeNode(min); */
/*     } */
/*     else{ */
/*       min->size = size; */
/*       freeNode *newptr = min + total; */
/*       return newNode(min, newptr, diff); */
/*     } */
/*   } */
/* } */

/* void my_free(void *ptr){ */
/*     if(ptr==NULL) return; */
/*     freeNode *blockStart = (freeNode *)(ptr - metasize); */
/*     //printf("ptr: %p, bs: %p, allocated size: %lu\n",ptr,blockStart,blockStart->size); */
/*     assert(blockStart->next == NULL && blockStart->prev == NULL); */
/*     if(head==NULL && tail==NULL){ */
/*         head = blockStart; */
/*         tail = blockStart; */
/*     }else{ */
/*         freeNode *traversal = head; */
/*         while(traversal!=NULL && traversal < blockStart){ */
/*             traversal = traversal->next; */
/*         } */
/*         if(traversal==NULL){ */
/*             //append to tail */
/*             tail->next = blockStart; */
/*             blockStart->prev = tail; */
/*             tail = blockStart; */
/*         }else if(traversal==head){ */
/*             //insert before traversal to be new head */
/*             blockStart->next = traversal; */
/*             traversal->prev = blockStart; */
/*             head = blockStart; */
/*         }else{ */
/*             //insert before traversal */
/*             blockStart->next = traversal; */
/*             blockStart->prev = traversal->prev; */
/*             blockStart->prev->next = blockStart; */
/*             traversal->prev = blockStart; */
/*         } */
/*     } */
/*     //    allocated -= blockStart->size+metasize; */
/*     merge(blockStart); */
/* } */

/* void merge(freeNode *block){ */
/*     if(block==NULL){ */
/*         return; */
/*     }else{ */
/*         freeNode *previous = block->prev; */
/*         freeNode *nextOne = block->next; */
/*         if(nextOne!=NULL && (void *)block + block->size + metasize == (void *)nextOne){ */
/*             block->size = block->size + nextOne->size + metasize; */
/*             block->next = nextOne->next; */
/*             if(block->next!=NULL){ */
/*                 block->next->prev = block; */
/*             }else{ */
/*                 tail = block; */
/*             } */
/*         } */
/*         if(previous!=NULL && (void *)previous + previous->size + metasize == (void *)block){ */
/*             previous->size = previous->size + block->size + metasize; */
/*             previous->next = block->next; */
/*             if(previous->next!=NULL){ */
/*                 previous->next->prev = previous; */
/*             }else{ */
/*                 tail = previous; */
/*             } */
/*         } */
/*     } */
/* } */


/* void ff_free(void *ptr){my_free(ptr);} */
/* void bf_free(void *ptr){my_free(ptr);} */

void printList() {
  freeNode *traversal = head;
  while (traversal != NULL) {
    printf("curr: %p, allocated size: %lu, allocated address: %p\n", traversal, traversal->size,(void *)traversal+metasize);
    traversal = traversal->next;
  }
}

void general_free(void *ptr){
  freeNode **cur = &head;
  freeNode *free_ptr = (freeNode *)(ptr - metasize);
  //empty freelist
  if(head == NULL){
    head = free_ptr;
    tail = free_ptr;
  }
  else{
    //find a position to add the node into the freelist
    while((*cur) != NULL && (*cur) < free_ptr){
      cur = &((*cur)->next);
    }
    //the position is before the head
    if((*cur) == head){
      free_ptr->next = (*cur);
      free_ptr->prev = NULL;
      (*cur)->prev = free_ptr;
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
      free_ptr->prev = (*cur)->prev;
      free_ptr->prev->next = free_ptr;
      (*cur)->prev = free_ptr;
    }
  }

  if(free_ptr == NULL){
    return;
  }
  else{
    //tell if it's needed to merge or not
    //merge with the later one
    if( free_ptr->next != NULL && (void*)free_ptr->next == (void *)free_ptr + metasize + free_ptr->size){
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

    //merge with the former one
    if(free_ptr->prev != NULL && (void*)free_ptr == (void*)free_ptr->prev + free_ptr->prev->size + metasize){
      free_ptr->prev->size = free_ptr->size + free_ptr->prev->size + metasize;
      freeNode *temp = free_ptr;
      free_ptr->prev->next = free_ptr->next;
      if(free_ptr->prev->next == NULL){
	tail = free_ptr->prev;
      }
      else{
	free_ptr->next->prev = free_ptr->prev;
      }
      temp->next = NULL;
      temp->prev = NULL;
    }
  }
 }


void ff_free(void *ptr){
  general_free(ptr);
}

void bf_free(void *ptr){
  general_free(ptr);
}



