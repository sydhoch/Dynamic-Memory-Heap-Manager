#include <stdio.h>  // needed for size_t
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;
void* dmalloc(size_t numbytes) {
  /* initialize through mmap call first time */
  if(freelist == NULL) { 			
    if(!dmalloc_init())
      return NULL;
  }

  assert(numbytes > 0);
  size_t anumbytes = ALIGN(numbytes);

  metadata_t *temp = freelist;
  //print_freelist();
  while(temp!=NULL){
    //if block is bigger than space needed
    if(temp->size > (anumbytes + METADATA_T_ALIGNED)){
      metadata_t *give = temp;

      //fix free block
      //get size for block to be reinserted into list
      size_t new_size = temp->size - (anumbytes + METADATA_T_ALIGNED);
      //move pointer of block to be reinserted
      metadata_t *insert_block = ((void*)temp) + anumbytes + METADATA_T_ALIGNED;
      insert_block->size = new_size;
      insert_block->next = temp->next;
      insert_block->prev = temp->prev;
      if (temp->next != NULL){
        (temp->next)->prev = insert_block;
      }
      if (temp->prev != NULL){
        (temp->prev)->next = insert_block;
      }
      if (temp->prev == NULL){
        freelist = insert_block;
      }
      give->size = anumbytes;
      give->next = NULL;
      give->prev = NULL;
      return ((void*)give) + METADATA_T_ALIGNED;
    }
    //if block = size needed
    if(temp->size == (anumbytes + METADATA_T_ALIGNED)){
      metadata_t *give = temp;
      metadata_t *original_next = temp->next;
      metadata_t *original_prev = temp->prev;
      if (temp->next == NULL && temp->prev == NULL){
        freelist = -1;
      }
      else if (original_prev == NULL){
        original_next->prev = NULL;
        freelist = original_next;
      }
      else if (original_next == NULL){
        original_prev->next = NULL;
      }
      else{
        original_prev->next = original_next;
        original_next->prev = original_prev;
      }
      give->next = NULL;
      give->prev = NULL;
      return ((void*)give) + METADATA_T_ALIGNED;
    }
    //check next block
    else{
      temp = temp->next;
    }
  }
	
  return NULL;
}

void dfree(void* ptr) {
  //if ptr == NULL, no operation is performed
  if (ptr == NULL){
    return;
  }
  metadata_t *rp = ptr - METADATA_T_ALIGNED;
  metadata_t *temp = freelist;
  
  while (temp != NULL){
    //if temp is only block
    if (temp->next == NULL && temp->prev == NULL){
      if (rp<temp){
        rp->next = temp;
        rp->prev = NULL;
        temp->prev = rp;
        freelist = rp;
        break;
      }
      if (rp>temp){
        temp->next = rp;
        rp->prev = temp;
        rp->next = NULL;
        break;
      }
    }
    //if temp is first
    if (temp->prev == NULL && temp->next !=NULL){
      if (rp<temp){
        rp->next = temp;
        rp->prev = NULL;
        temp->prev = rp;
        freelist = rp;
        break;
      }
    }
    //if temp is last
    if (temp->prev != NULL && temp->next == NULL){
      if (rp<temp){
        temp->prev->next = rp;
        rp->prev = temp->prev;
        rp->next = temp;
        temp->prev = rp;
        break;
      }
      if (rp>temp){
        temp->next = rp;
        rp->prev = temp;
        rp->next = NULL;
        break;
      }
    }
    // if temp is in the middle
    if (temp->prev != NULL && temp->next != NULL){
      metadata_t *original_prev = temp->prev;
      metadata_t *original_next = temp->next;
      if (rp<temp){
        original_prev->next = rp;
        rp->prev = original_prev;
        temp->prev = rp;
        rp->next = temp;
        break;
      }
    }
    temp = temp->next;
  }

  metadata_t *curr = freelist;
  while(curr->next != NULL){
    while (((void*)curr) +METADATA_T_ALIGNED + curr->size == curr->next){
      size_t new_size = curr->size + METADATA_T_ALIGNED + curr->next->size;
      curr->size = new_size;
      curr->next = curr->next->next;
      if (curr->next!=NULL){
        curr->next->prev = curr;
      }
    }
    if (curr->next !=NULL){
      curr = curr->next;
    }
    else{
      break;
    }
  }

}

bool dmalloc_init() {

  /*  Initialize freelist pointers to NULL
   */

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}

void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
