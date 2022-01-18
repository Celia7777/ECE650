#include "my_malloc.h"


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
  freeNode **cur = &head;
  size_t total = size + metasize;
  //empty heap
  if((*cur) == NULL){
    void *brkp = sbrk(total);
    void *addr = brkp + metasize;
    return addr;
  }
  while((*cur) != NULL){
    //find a matched position to allocate mem
    if((*cur)->size == size){
      //remove this node from freelist
      return removeNode((*cur));
    }
    else if((*cur)->size > size){
      //need to split
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
	freeNode *newptr = (*cur) + total;
	return newNode((*cur), newptr, diff);
	/* freeNode *ptr = (*cur) + total; */
	/* if((*cur)->next == NULL){ */
	/*   tail = ptr; */
	/* } */
	/* else{ */
	/*   (*cur)->next->prev = ptr;  */
	/* } */
	/* if((*cur)->prev == NULL){ */
	/*   head = ptr; */
	/* } */
	/* else{ */
	/*   (*cur)->prev->next = ptr; */
	/* } */
	/* ptr->size = diff; */
	/* ptr->next = (*cur)->next; */
	/* ptr->prev = (*cur)->prev; */
	/* (*cur)->next = NULL; */
	/* (*cur)->prev = NULL; */
	/* void *addr = (void *)(*cur) + metasize; */
	/* return addr; */
      }
    }
    else{
      cur = &(*cur)->next;
    }
  }
  //no matched mem, need to expand heap
  void *break_exp = sbrk(total);
  void *addr = (void *)break_exp + metasize;
  return addr;
}

void *bf_malloc(size_t size){
  freeNode **cur = &head;
  size_t total = size + metasize;
  //empty heap
  if((*cur) == NULL){
    void *brkp = sbrk(total);
    void *addr = brkp + metasize;
    return addr;
  }
  //use a pointer to record the min mem
  freeNode *min = NULL;
  while((*cur) != NULL){
    //find a matched position to allocate mem
    if((*cur)->size == size){
      min = (*cur);
      return removeNode((*cur));
    }
    else if((*cur)->size > size){
      if((*cur)->size <= min->size){
	min = (*cur);
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
    void *break_exp = sbrk(total);
    void *addr = (void *)break_exp + metasize;
    return addr;
  }
  else{
    size_t diff = min->size - size;
    //remaining size cannot hold metadata
    if(diff <= metasize){
      return removeNode(min);
    }
    else{
      min->size = size;
      freeNode *newptr = min + total;
      return newNode(min, newptr, diff);
    }
  }
}

void my_free(void *ptr){
    if(ptr==NULL) return;
    block_t *blockStart = (block_t *)(ptr - block_size);
    //printf("ptr: %p, bs: %p, allocated size: %lu\n",ptr,blockStart,blockStart->size);
    assert(blockStart->next == NULL && blockStart->prev == NULL);
    if(head==NULL && tail==NULL){
        head = blockStart;
        tail = blockStart;
    }else{
        block_t *traversal = head;
        while(traversal!=NULL && traversal < blockStart){
            traversal = traversal->next;
        }
        if(traversal==NULL){
            //append to tail
            tail->next = blockStart;
            blockStart->prev = tail;
            tail = blockStart;
        }else if(traversal==head){
            //insert before traversal to be new head
            blockStart->next = traversal;
            traversal->prev = blockStart;
            head = blockStart;
        }else{
            //insert before traversal
            blockStart->next = traversal;
            blockStart->prev = traversal->prev;
            blockStart->prev->next = blockStart;
            traversal->prev = blockStart;
        }
    }
    allocated -= blockStart->size+block_size;
    merge(blockStart);
}

void merge(block_t *block){
    if(block==NULL){
        return;
    }else{
        block_t *previous = block->prev;
        block_t *nextOne = block->next;
        if(nextOne!=NULL && (void *)block + block->size + block_size == (void *)nextOne){
            block->size = block->size + nextOne->size + block_size;
            block->next = nextOne->next;
            if(block->next!=NULL){
                block->next->prev = block;
            }else{
                tail = block;
            }
        }
        if(previous!=NULL && (void *)previous + previous->size + block_size == (void *)block){
            previous->size = previous->size + block->size + block_size;
            previous->next = block->next;
            if(previous->next!=NULL){
                previous->next->prev = previous;
            }else{
                tail = previous;
            }
        }
    }
}

void ff_free(void *ptr){my_free(ptr);}
void bf_free(void *ptr){my_free(ptr);}


/* void general_free(void *ptr){ */
/*   freeNode **cur = &head; */
/*   freeNode *free_ptr = (freeNode *)(ptr - metasize); */
/*   //empty freelist */
/*   if(head == NULL){ */
/*     head = free_ptr; */
/*     tail = free_ptr; */
/*   } */
/*   //find a position to add the node into the freelist */
/*   while((*cur) != NULL && (*cur) < free_ptr){ */
/*     cur = &(*cur)->next; */
/*   } */
/*   //the position is before the head */
/*   if((*cur) == head){ */
/*     free_ptr->next = (*cur); */
/*     free_ptr->prev = NULL; */
/*     (*cur)->prev = free_ptr; */
/*     head = free_ptr; */
/*   } */
/*   //the position is after the tail */
/*   else if((*cur) == NULL){ */
/*     free_ptr->next = NULL; */
/*     free_ptr->prev = tail; */
/*     tail->next = free_ptr; */
/*     tail = free_ptr; */
/*   } */
/*   //the position is at the medium */
/*   else{ */
/*     free_ptr->next = (*cur); */
/*     free_ptr->prev = (*cur)->prev; */
/*     free_ptr->prev->next = free_ptr; */
/*     (*cur)->prev = free_ptr; */
/*   } */
  
/*   //tell if it's needed to merge or not */
/*   //merge with the later one */
/*   if(free_ptr->next == free_ptr + metasize + free_ptr->size){ */
/*     free_ptr->size = free_ptr->size + free_ptr->next->size + metasize; */
/*     freeNode *temp = free_ptr->next; */
/*     free_ptr->next = free_ptr->next->next; */
/*     free_ptr->next->prev = free_ptr; */
/*     temp->next = NULL; */
/*     temp->prev = NULL; */
/*   } */
/*   //merge with the former one */
/*   if(free_ptr == free_ptr->prev + free_ptr->prev->size + metasize){ */
/*     //free the tail one */
/*     if(free_ptr->next == NULL){ */
/*       free_ptr->size = free_ptr->size + free_ptr->prev->size + metasize; */
/*       free_ptr->prev->next = NULL; */
/*       tail = free_ptr->prev; */
/*       free_ptr->prev = NULL; */
/*     } */
/*     else{ */
/*       free_ptr->size = free_ptr->size + free_ptr->prev->size + metasize; */
/*       freeNode *temp = free_ptr; */
/*       free_ptr->prev->next = free_ptr->next; */
/*       free_ptr->next->prev = free_ptr->prev; */
/*       temp->next = NULL; */
/*       temp->prev = NULL; */
/*     } */
/*   } */
 
/*  } */

/* void ff_free(void *ptr){ */
/*   general_free(ptr); */
/* } */

/* void bf_free(void *ptr){ */
/*   general_free(ptr); */
/* } */

/* int main() { */
/*   void* br[4] = {0}; */
/*   br[0] = sbrk(0); */
/*   br[1] = sbrk(10); */
/*   br[2] = sbrk(0); */
/*   br[3] = sbrk(5); */
/*   for(int i = 0; i < 4; i++) { */
/*     printf("end of the break: %p\n", br[i]); */
/*   } */
/* } */

