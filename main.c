#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "malloc_free.h"
#include "tests.h"

void scan_free_list()
{
    printf("\nSCANNING FREE LIST\n");

    node_t *curr = start_of_free_list;

    while (curr)
    {
        printf("Free chunk at %llu with size %llu and next %llu\n", (uint64_t)curr - start, (uint64_t)curr->size, curr->next ? (uint64_t)curr->next - start : 0);
        curr = curr->next;
    }
}

void scan_allocated_list()
{

    printf("\nSCANNING ALLOCATED LIST\n");

    void *ptr = start_of_heap;
    node_t *free_block = start_of_free_list;

    while (ptr < start_of_heap + SIZE_OF_HEAP)
    {
        if (ptr == free_block)
        {
            node_t *free_chunk = (node_t *)ptr;

            free_block = free_block->next;

            ptr += (free_chunk->size + sizeof(node_t));
        }
        else
        {
            header_t *chunk = (header_t *)ptr;

            assert(chunk->magic == MAGIC_NUMBER);

            printf("Allocated chunk at %llu with size %zu and magic %d\n", (uint64_t)chunk - start, chunk->size, chunk->magic);

            ptr += (chunk->size + sizeof(header_t));
        }
    }
}

void audit()
{
    printf("\nAUDITING THE HEAP\n\n");

    void *ptr = start_of_heap;
    node_t *free_block = start_of_free_list;

    while (ptr < start_of_heap + SIZE_OF_HEAP)
    {
        // if ptr matches free block then
        // segment must be free
        if (ptr == free_block)
        {
            // cast the ptr to a free node
            node_t *free_chunk = (node_t *)ptr;

            printf("\x1b[34m");
            printf("--------------------\n");
            printf("FREE BLOCK\n");
            printf("ADDRESS: %llu\n", (uint64_t)ptr - start);
            printf("SIZE: %ld\n", free_chunk->size);
            printf("NEXT: %llu\n", free_chunk->next ? (uint64_t)free_chunk->next - start : 0);
            printf("--------------------\n");
            printf("\x1b[1m");
            printf("\x1b[0m");
            // printf("Size of offset: %llu\n", start);

            free_block = free_block->next;
            ptr += (free_chunk->size + sizeof(node_t));
        }
        // segment must be allocated
        else
        {
            header_t *chunk = (header_t *)ptr;
            assert(chunk->magic == MAGIC_NUMBER);

            printf("\x1b[31m");
            printf("--------------------\n");
            printf("ALLOCATED BLOCK\n");
            printf("ADDRESS: %llu\n", (uint64_t)ptr - start);
            printf("SIZE: %ld\n", chunk->size);
            printf("--------------------\n");
            printf("\x1b[1m");
            printf("\x1b[0m");
            ptr += (chunk->size + sizeof(header_t));
        }
    }

    assert((uint64_t)ptr - start == SIZE_OF_HEAP);
}

void display_commands()
{
    printf("\nCommands:\n");
    printf("\nmalloc - allocate n number of bytes");
    printf("\nfree - free allocated block at address");
    printf("\naudit- account for all memory on the heap");
    printf("\nscan_free - show all free spaces on heap");
    printf("\nscan_alloc - show all allocated blocks on heap");
    printf("\ntests - display a list of tests to run");
    printf("\nhelp - display a list of commands");
    printf("\nexit - End shell session");
}

void show_tests()
{
    printf("             AVAILABLE TESTS             \n");
    printf("=========================================\n");
    printf("Please note: running a test will clear the heap\n\n");
    printf("all - run all tests in below order\n");
    printf("reuse - run free chunk reuse tests\n");
    printf("sorted - run sorted free list tests\n");
    printf("splitting - run splitting free chunks tests\n");
    printf("coalescing - run coalescing tests\n");
    printf("alternating - run alternating sequence tests\n");
    printf("fit - run worst fit tests\n");
    printf("return - run malloc bad value tests\n\n");
}

/* Run the selected test. */
void select_test(char *which)
{
    if (!strcmp(which, "all"))
    {
        test_all();
    }
    else if (!strcmp(which, "reuse"))
    {
        test_free_chunk_reuse();
    }
    else if (!strcmp(which, "sorted"))
    {
        test_sorted_free_list();
    }
    else if (!strcmp(which, "splitting"))
    {
        test_splitting_free_chunks();
    }
    else if (!strcmp(which, "coalescing"))
    {
        test_coalesce();
    }
    else if (!strcmp(which, "alternating"))
    {
        test_alternating_sequence();
    }
    else if (!strcmp(which, "fit"))
    {
        test_worst_fit();
    }
    else if (!strcmp(which, "return"))
    {
        test_malloc_bad_size();
    }
    else
    {
        printf("Unrecognized test selection. Type 'tests' to see the list of available tests\n");
    }
}

void begin_shell()
{
    char comm[100];
    display_commands();

    while (strcmp(comm, "exit"))
    {
        printf("\n> ");
        scanf("%s", comm);

        if (!strcmp(comm, "audit"))
        {
            audit();
        }

        else if (!strcmp(comm, "scan_free"))
        {
            scan_free_list();
        }

        else if (!strcmp(comm, "scan_alloc"))
        {
            scan_allocated_list();
        }

        else if (!strcmp(comm, "malloc"))
        {
            int size;
            printf("\nbytes to allocate: ");
            scanf("%d", &size);
            my_malloc(size);
        }

        else if (!strcmp(comm, "free"))
        {
            int address;
            printf("starting address of allocated chunk you would like to free: ");
            scanf("%d", &address);

            header_t *addr = (header_t *)(address + start);
            if (addr->magic == MAGIC_NUMBER)
            {
                my_free(addr + 1);
            }
            else
            {
                printf("%d is not a valid address", address);
            }
        }
        else if (!strcmp(comm, "tests"))
        {
            show_tests();
            char which[20];
            printf("Which test to run: ");
            scanf("%s", which);
            select_test(which);
        }
        else if (!strcmp(comm, "help"))
        {
            display_commands();
        }
        else if (strcmp(comm, "exit"))
        {
            printf("not a valid command. 'help' will show you a list of valid commands\n");
        }
    }
    printf("\nSession Terminated\n");
}

int main(int argc, char const *argv[])
{
    init_heap();
    init_tests();
    begin_shell();
    return 0;
}