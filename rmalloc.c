
#include "rmalloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

block_meta *heap_head = NULL;

block_meta *find_free_block(struct block_meta **last, size_t size) {
  struct block_meta *current = heap_head;

  while (current && !(current->free && current->size >= size)) {
    *last = current;
    current = current->next;
  }
  return current;
}

void print_heap() {
  struct block_meta *test_block = heap_head;

  printf("HEAP: ");
  if (test_block == NULL) {
    printf("(empty)\n");
    return;
  }
  while (test_block != NULL) {
    printf("[%p, size: %zu, free: %d] -> ", (void *)test_block,
           test_block->size, test_block->free);

    test_block = test_block->next;
  }

  printf("NULL\n");
}

block_meta *request_space(block_meta *last, size_t size) {
  struct block_meta *block;

  block = sbrk(0);
  void *request = sbrk(size + META_SIZE);
  assert((void *)block == request);
  if (request == (void *)-1) {
    return NULL; // sbrk failed.
  }

  if (block == (void *)-1) {
    return NULL;
  }

  if (last) {
    last->next = block;
    block->prev = last;
  } else {
    block->prev = NULL;
  }

  block->size = size;
  block->next = NULL;
  block->free = 0;

  return block;
}

void split_chunk(block_meta *block, size_t size) {
  if (block->size >= size + META_SIZE + sizeof(void *)) {
    block_meta *new_block = (block_meta *)((char *)(block + 1) + size);

    new_block->size = block->size - size - META_SIZE;
    new_block->next = block->next;
    new_block->free = 1;

    new_block->prev = block;

    if (new_block->next != NULL) {
      new_block->next->prev = new_block;
    }

    block->size = size;
    block->next = new_block;
  }
}

block_meta *coalecesce_chunck(struct block_meta *block) {

  if (block->next && block->next->free) {
    if ((char *)block + META_SIZE + block->size == (char *)block->next) {
      block->size += META_SIZE + block->next->size;
      block->next = block->next->next;

      if (block->next) {
        block->next->prev = block;
      }
    }
  }

  if (block->prev && block->prev->free) {
    if ((char *)block->prev + META_SIZE + block->prev->size == (char *)block) {
      block->prev->size += META_SIZE + block->size;
      block->prev->next = block->next;

      if (block->next) {
        block->next->prev = block->prev;
      }

      block = block->prev;
    }
  }
  return block;
}

struct block_meta *get_block_ptr(void *ptr) {
  return (struct block_meta *)ptr - 1;
}

void *rmalloc(size_t size) {
  struct block_meta *block;
  if (size <= 0) {
    return NULL;
  }
  size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

  if (!heap_head) {
    block = request_space(NULL, size);
    if (!block)
      return NULL;
    heap_head = block;
  } else {
    struct block_meta *last = heap_head;
    block = find_free_block(&last, size);

    if (!block) {
      block = request_space(last, size);
      if (!block)
        return NULL;
    } else {
      split_chunk(block, size);
      block->free = 0;
    }
  }

  printf("MALLOC\n");
  print_heap();

  return (block + 1);
}

void rfree(void *ptr) {
  if (!ptr) {
    return;
  }
  struct block_meta *block_ptr = get_block_ptr(ptr);

  assert(block_ptr->free == 0);
  block_ptr->free = 1;

  block_meta *last_block = coalecesce_chunck(block_ptr);

  printf("FREE\n");
  print_heap();

}

void *rrealloc(void *ptr, size_t size) {
  if (!ptr) {
    return rmalloc(size);
  }

  struct block_meta *block_ptr = get_block_ptr(ptr);
  if (block_ptr->size >= size) {
    return ptr;
  }
  void *new_ptr;
  new_ptr = rmalloc(size);

  if (!new_ptr) {
    return NULL;
  }

  memcpy(new_ptr, ptr, block_ptr->size);
  rfree(ptr);
  return new_ptr;
}

void *rcalloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize; // TODO: check for overflow.
  void *ptr = rmalloc(size);

  memset(ptr, 0, size);

  return ptr;
}
