//research
#pragma once

#include <stddef.h> 

typedef struct block_meta {
    size_t size;
    int free;
    struct block_meta *next;
    struct block_meta *prev;
} block_meta;

#define META_SIZE sizeof(struct block_meta)

void *rmalloc(size_t size);
void rfree(void *ptr);
void *rrealloc(void *ptr, size_t size);
void *rcalloc(size_t nelem, size_t elsize);
