#include "malloc_free.h"

const size_t SIZE_OF_HEAP = 4096;
const int MAGIC_NUMBER = 123456789;
const size_t ALIGN_TO = 8;

size_t align(size_t raw)
{
    size_t aligned = ALIGN_TO * ((raw - 1 + ALIGN_TO + sizeof(header_t)) / ALIGN_TO);
    return aligned;
}

void coalesce()
{
    node_t *curr = start_of_free_list;
    while (curr)
    {
        if ((uint64_t)curr + sizeof(node_t) + curr->size == (uint64_t)curr->next)
        {
            // next item in heap = free block
            node_t *temp = curr->next;

            curr->next = temp->next;
            curr->size = curr->size + temp->size + sizeof(node_t);
        }
        // Skip to next node_t if we didn't merge anything
        else
        {
            curr = curr->next;
        }
    }
}

/* Returns pointer to memory. Returns NULL if there is not enough space. */
void *my_malloc(size_t size)
{
    // If there are no free chunks
    if (!start_of_free_list)
    {
        printf("There are no free chunks!\n");
        return NULL;
    }

    // If they enter a negative number the size will overflow to the max integer so this will fire
    if (size > SIZE_OF_HEAP)
    {
        printf("REQUESTED SIZE EXCEEDS HEAP SIZE\n");
        printf("Did you try to allocate a negative size?\n");
        return NULL;
    }
    // Not sure if this is supposed to happen but it makes sense to deny a request of size 0
    else if (size == 0)
    {
        printf("REFUSING TO ALLOCATE SIZE 0\n");
        return NULL;
    }

    size_t needed_size = align(size);

    // WORST FIT
    // Search for biggest chunk for worst fit
    node_t *curr = start_of_free_list;
    node_t *biggest_chunk_prev = curr;
    node_t *biggest_chunk = curr;
    while (curr)
    {
        if (curr->next && curr->next->size > biggest_chunk->size)
        {
            biggest_chunk_prev = curr;
            biggest_chunk = curr->next;
        }
        curr = curr->next;
    }

    // If there is no chunk big enough return NULL
    if (needed_size > biggest_chunk->size + sizeof(node_t))
    {
        printf("NO CHUNK BIG ENOUGH\n");
        return NULL;
    }

    size_t prev_size = biggest_chunk->size;
    node_t *prev_next = biggest_chunk->next;

    // Split free chunk
    // If the chunk to be split is the head
    if (biggest_chunk == start_of_free_list)
    {
        // If the needed size requires the overhead space too
        if (needed_size > biggest_chunk->size)
        {
            start_of_free_list = biggest_chunk->next;
        }
        else
        {
            start_of_free_list = (node_t *)((char *)biggest_chunk + needed_size);
            start_of_free_list->size = prev_size - needed_size;
            start_of_free_list->next = prev_next;
        }
    }
    else
    {
        // If the needed size requires the overhead space too
        if (needed_size > biggest_chunk->size)
        {
            biggest_chunk_prev->next = biggest_chunk->next;
        }
        else
        {
            node_t *split_free_chunk = (node_t *)((char *)biggest_chunk + needed_size);
            split_free_chunk->size = prev_size - needed_size;
            biggest_chunk_prev->next = split_free_chunk;
        }
    }

    // Create header_t
    header_t *allocated_header_t = (header_t *)biggest_chunk;
    allocated_header_t->size = needed_size - sizeof(header_t);
    allocated_header_t->magic = MAGIC_NUMBER;

    // Cut big chunk down to size
    header_t *allocated_address = (header_t *)biggest_chunk + 1;

    return (void *)allocated_address;
}

/* Frees the allocated chunk starting at the pointer passed in. Orders and coalesces the free list afterwards. */
void my_free(void *ptr)
{
    header_t *hptr = (header_t *)ptr - 1;
    assert(hptr->magic == MAGIC_NUMBER);
    node_t *new_free_chunk = (node_t *)hptr;

    // Edge case where there is no free list
    if (!start_of_free_list)
    {
        start_of_free_list = new_free_chunk;
        start_of_free_list->next = NULL;
        start_of_free_list->size = hptr->size - sizeof(header_t) + sizeof(node_t);
        return;
    }

    // Insert into free list at the right place so need for reordering
    node_t *prev = start_of_free_list;
    node_t *curr = start_of_free_list;

    // Loop through list to find correct placement
    while (curr < new_free_chunk)
    {
        prev = curr;
        curr = curr->next;
    }

    // If it is at the start
    if (curr == start_of_free_list)
    {
        // Add new free chunk to head of free list
        new_free_chunk->next = start_of_free_list;
        new_free_chunk->size = hptr->size - sizeof(header_t) + sizeof(node_t);
        start_of_free_list = new_free_chunk;
    }
    else
    {
        prev->next = new_free_chunk;
        new_free_chunk->next = curr;
        new_free_chunk->size = hptr->size - sizeof(header_t) + sizeof(node_t);
    }

    // Try to coalesce
    coalesce();
}

void init_heap()
{
    start_of_heap = mmap(NULL, SIZE_OF_HEAP, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    start = (uint64_t)start_of_heap;

    start_of_free_list = (node_t *)start_of_heap;
    start_of_free_list->size = SIZE_OF_HEAP - sizeof(node_t);
    start_of_free_list->next = NULL;

    printf("Heap initialized at address: %llu with size: %ld\n", (uint64_t)start_of_heap - start, SIZE_OF_HEAP);
}