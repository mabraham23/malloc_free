#ifndef _MALLOC_FREE_H_
#define _MALLOC_FREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <assert.h>

extern const size_t SIZE_OF_HEAP;
extern const int MAGIC_NUMBER;
extern const size_t ALIGN_TO;

typedef struct __header_t
{
    size_t size;
    int magic;
} header_t;

typedef struct __node_t
{
    size_t size;
    struct __node_t *next;
} node_t;

void *start_of_heap;
node_t *start_of_free_list;
uint64_t start;

size_t align(size_t raw);
void coalesce();
void *my_malloc(size_t size);
void my_free(void *ptr);
void init_heap();

#endif