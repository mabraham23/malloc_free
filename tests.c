#include <stdbool.h>
#include "malloc_free.h"
#include "main.h"
#include "tests.h"

#pragma region Test_Helpers

/* Adds emphasis */
void emphasis(char *message)
{
    char top[100] = {0};
    char bot[100] = {0};
    for (size_t i = 0; i < strlen(message); i++)
    {
        strcat(top, "=");
        strcat(bot, "=");
    }
    printf("\n%s\n%s\n%s\n\n", top, message, bot);
}

/* Just colors it green to indicate success */
void success(char *message)
{
    printf("\x1b[32m");
    emphasis(message);
    printf("\x1b[0m");
}

/* Just so if I want to edit this it is easier */
void passed()
{
    printf("\nTEST PASSED\n\n");
}

/* Frees any allocated chunks on the heap. */
void free_all_chunks()
{
    void *chunks_to_free[MAX_CHUNKS];
    int num_allocated_chunks = 0;

    void *address = start_of_heap;
    node_t *last_free = start_of_free_list;
    while (address < start_of_heap + SIZE_OF_HEAP)
    {
        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            node_t *chunk = (node_t *)address;

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node_t));
        }
        // else it must be allocated
        else
        {
            header_t *chunk = (header_t *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            // can't free while inside this loop, so store the address for later
            chunks_to_free[num_allocated_chunks] = chunk + 1;
            num_allocated_chunks++;

            // next chunk
            address += (chunk->size + sizeof(header_t));
        }
    }
    // Free them all
    for (size_t i = 0; i < num_allocated_chunks; i++)
    {
        my_free(chunks_to_free[i]);
    }
}

/* Verifies that the each free list node_t's next pointer address is greater than the node_t's current address. */
bool verify_sorted()
{
    bool sorted = true;
    node_t *curr = start_of_free_list;
    while (curr)
    {
        if (curr->next && curr->next <= curr)
        {
            sorted = false;
        }

        curr = curr->next;
    }

    return sorted;
}

/* Verifies that no free list node_ts next pointer is equal to the address of the node_t plus it's size, which woullu mean they are adjacent. */
bool verify_alternating()
{
    bool alternating = true;
    node_t *curr = start_of_free_list;
    while (curr)
    {
        if ((uint64_t)curr + curr->size + sizeof(node_t) == (uint64_t)curr->next)
        {
            alternating = false;
        }

        curr = curr->next;
    }

    return alternating;
}

#pragma endregion Test_Helpers

#pragma region Tests

void test_free_chunk_reuse()
{
    emphasis("TESTING FREE CHUNKS BEING REUSED AS MUCH AS POSSIBLE");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 1 CHUNK WITH SIZE 1/3 OF HEAP...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP / 3);
    printf("ALLOCATING 1 CHUNK WITH A SLIGHTLY GREATER SIZE...\n");
    chunks[1] = my_malloc((SIZE_OF_HEAP / 3) + CHUNK_SIZE);
    printf("FREEING THE FIRST CHUNK...\n");
    my_free(chunks[0]);
    printf("ALLOCATING ANOTHER CHUNK...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING ADDRESS OF FIRST CHUNK AT START OF HEAP...\n");
    audit();
    assert((header_t *)chunks[0] - 1 == start_of_heap);
    free_all_chunks();
    passed();

    printf("ALLOCATING 2 CHUNKS EACH WITH SIZE 1/3 OF HEAP...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP / 3);
    chunks[1] = my_malloc(SIZE_OF_HEAP / 3);
    printf("ALLOCATING ANOTHER CHUNK OF STANDARD SIZE...\n");
    chunks[2] = my_malloc(CHUNK_SIZE);
    printf("FREEING THE FIRST 2 CHUNKS...\n");
    my_free(chunks[0]);
    my_free(chunks[1]);
    printf("ALLOCATING ANOTHER CHUNK...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING ADDRESS OF FIRST CHUNK AT START OF HEAP...\n");
    audit();
    assert((header_t *)chunks[0] - 1 == start_of_heap);
    free_all_chunks();
    passed();

    printf("ALLOCATING 3 CHUNKS EACH WITH SIZE 1/4 OF HEAP...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP / 4);
    chunks[1] = my_malloc(SIZE_OF_HEAP / 4);
    chunks[2] = my_malloc(SIZE_OF_HEAP / 4);
    printf("ALLOCATING ANOTHER CHUNK OF STANDARD SIZE...\n");
    chunks[3] = my_malloc(CHUNK_SIZE);
    printf("FREEING THE SECOND CHUNK...\n");
    my_free(chunks[1]);
    printf("VERIFYING THAT FREE LIST HEAD IS AT THE END OF FIRST ALLOCATED CHUNK...\n");
    audit();
    assert(start_of_free_list == start_of_heap + align(SIZE_OF_HEAP / 4));
    printf("ALLOCATING ANOTHER CHUNK...\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING THAT NEW CHUNK ADDRESS IS AT THE END OF FIRST ALLOCATED CHUNK...\n");
    audit();
    assert((header_t *)chunks[1] - 1 == start_of_heap + align(SIZE_OF_HEAP / 4));
    free_all_chunks();
    passed();

    success("ALL FREE CHUNK REUSE TESTS PASSED");
}

void test_sorted_free_list()
{
    emphasis("TESTING FREE LIST IS SORTED IN IN-MEMORY ORDER");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 5 CHUNKS...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING EVERY OTHER CHUNK...\n");
    my_free(chunks[0]);
    my_free(chunks[2]);
    my_free(chunks[4]);
    printf("VERIFYING THAT THE FREE CHUNK LIST IS SORTED...\n");
    audit();
    assert(verify_sorted());
    free_all_chunks();
    passed();

    printf("ALLOCATING 10 CHUNKS...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    chunks[6] = my_malloc(CHUNK_SIZE);
    chunks[7] = my_malloc(CHUNK_SIZE);
    chunks[8] = my_malloc(CHUNK_SIZE);
    chunks[9] = my_malloc(CHUNK_SIZE);
    printf("FREEING EVERY OTHER CHUNK IN A NON-SEQUENTIAL ORDER...\n");
    my_free(chunks[4]);
    my_free(chunks[6]);
    my_free(chunks[2]);
    my_free(chunks[0]);
    my_free(chunks[8]);
    printf("VERIFYING THAT THE FREE CHUNK LIST IS SORTED...\n");
    audit();
    assert(verify_sorted());
    free_all_chunks();
    passed();

    success("ALL SORTED FREE LIST TESTS PASSED");
}

void test_splitting_free_chunks()
{
    emphasis("TESTING FREE CHUNKS ARE SPLIT PROPERLY WHEN ALLOCATING");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 1 CHUNK...\n");
    node_t *prev_head_address = start_of_free_list;
    chunks[0] = my_malloc(CHUNK_SIZE);
    uint64_t expected = (uint64_t)prev_head_address + align(CHUNK_SIZE);
    printf("CHECKING ADDRESS OF FREE LIST HEAD...\n");
    printf("EXPECTED: %llu, ACTUAL: %llu\n", expected - start, (uint64_t)start_of_free_list - start);
    assert((uint64_t)start_of_free_list == expected);
    free_all_chunks();
    passed();

    prev_head_address = start_of_free_list;
    printf("ALLOCATING 1 CHUNK OF SIZE 1/2 OF HEAP SIZE...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP / 2);
    printf("ALLOCATING ANOTHER CHUNK OF STANDARD SIZE...\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING THAT FREE LIST HEAD HAS MOVED UP BY TOTAL SIZE OF ALLOCATED CHUNKS...\n");
    expected = (uint64_t)prev_head_address + align(SIZE_OF_HEAP / 2) + align(CHUNK_SIZE);
    printf("CHECKING ADDRESS OF FREE LIST HEAD...\n");
    printf("EXPECTED: %llu, ACTUAL: %llu\n", expected - start, (uint64_t)start_of_free_list - start);
    audit();
    assert((uint64_t)start_of_free_list == expected);
    printf("FREEING FIRST CHUNK...\n");
    my_free(chunks[0]);
    prev_head_address = start_of_free_list;
    printf("ALLOCATING ANOTHER CHUNK OF STANDARD SIZE...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING THAT FREE LIST HEAD HAS MOVED UP BY SIZE OF ALLOCATED CHUNK...\n");
    expected = (uint64_t)prev_head_address + align(CHUNK_SIZE);
    printf("CHECKING ADDRESS OF FREE LIST HEAD...\n");
    printf("EXPECTED: %llu, ACTUAL: %llu\n", expected - start, (uint64_t)start_of_free_list - start);
    audit();
    assert((uint64_t)start_of_free_list == expected);
    free_all_chunks();
    passed();

    printf("ALLOCATING 1 CHUNK THAT IS THE MAX CHUNK SIZE THE HEAP CAN HOLD...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP - sizeof(header_t));
    printf("VERIFYING FREE LIST HEAD IS NULL...\n");
    audit();
    assert(start_of_free_list == NULL);
    free_all_chunks();
    passed();

    printf("ALLOCATING 1 CHUNK OF SIZE 1/2 OF HEAP SIZE...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP / 2);
    printf("ALLOCATING ANOTHER CHUNK OF STANDARD SIZE...\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING FIRST CHUNK...\n");
    my_free(chunks[0]);
    printf("ALLOCATING 1 CHUNK OF SIZE 1/2 OF HEAP SIZE...\n");
    chunks[0] = my_malloc(SIZE_OF_HEAP / 2);
    printf("VERIFYING THAT THERE IS ONLY 1 FREE CHUNK\n");
    audit();
    assert(start_of_free_list->next == NULL);
    free_all_chunks();
    passed();

    success("ALL SPLITTING FREE CHUNKS TESTS PASSED");
}

void test_coalesce()
{
    emphasis("TESTING FREE CHUNKS BEING COALESCED PROPERLY");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 5 CHUNKS...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING ALL CHUNKS...\n");
    free_all_chunks();
    printf("MAKING SURE THERE IS ONLY 1 CHUNK...\n");
    audit();
    assert(start_of_free_list->next == NULL);
    passed();

    printf("ALLOCATING 5 CHUNKS...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING FIRST 2 AND LAST 2 CHUNKS...\n");
    my_free(chunks[0]);
    my_free(chunks[1]);
    my_free(chunks[3]);
    my_free(chunks[4]);
    printf("MAKING SURE THERE ARE ONLY 2 FREE CHUNKS...\n");
    audit();
    assert(start_of_free_list->next->next == NULL);
    free_all_chunks();
    passed();

    printf("ALLOCATING 5 CHUNKS...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING CHUNKS 1, 3 AND 4...\n");
    my_free(chunks[0]);
    my_free(chunks[2]);
    my_free(chunks[3]);
    printf("MAKING SURE THERE ARE ONLY 3 FREE CHUNKS...\n");
    audit();
    assert(start_of_free_list->next->next->next == NULL);
    free_all_chunks();
    passed();

    success("ALL COALESCING TESTS PASSED");
}

void test_alternating_sequence()
{
    emphasis("TESTING HEAP IS IN ALTERNATING SEQUENCE OF 1 FREE CHUNK AND 1 OR MORE ALLOCATED CHUNKS");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 2 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING SECOND CHUNK\n");
    my_free(chunks[1]);
    printf("VERIFYING NO FREE CHUNKS ARE NEXT TO EACH OTHER...\n");
    audit();
    assert(verify_alternating());
    free_all_chunks();
    passed();

    printf("ALLOCATING 7 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    chunks[6] = my_malloc(CHUNK_SIZE);
    printf("FREEING EVERY OTHER CHUNK\n");
    my_free(chunks[0]);
    my_free(chunks[2]);
    my_free(chunks[4]);
    my_free(chunks[6]);
    printf("VERIFYING NO FREE CHUNKS ARE NEXT TO EACH OTHER...\n");
    audit();
    assert(verify_alternating());
    free_all_chunks();
    passed();

    printf("ALLOCATING 3 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(SIZE_OF_HEAP / 2);
    chunks[2] = my_malloc(CHUNK_SIZE);
    printf("FREEING MIDDLE CHUNK\n");
    my_free(chunks[1]);
    printf("ALLOCATING 4 MORE CHUNKS\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING NO FREE CHUNKS ARE NEXT TO EACH OTHER...\n");
    audit();
    assert(verify_alternating());
    free_all_chunks();
    passed();

    success("ALL ALTERNATING SEQUENCE TESTS PASSED");
}

void test_worst_fit()
{
    emphasis("TESTING WORST FIRST ALLOCATION");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 2 CHUNKS...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING FIRST CHUNK...\n");
    my_free(chunks[0]);
    printf("ALLOCATING CHUNK OF SMALLER SIZE...\n");
    chunks[0] = my_malloc(CHUNK_SIZE / 2);
    printf("VERIFYING ADDRESS OF SECOND CHUNK IS GREATER THAN ADDRESS OF FIRST...\n");
    audit();
    assert(chunks[0] > chunks[1]);
    free_all_chunks();
    passed();

    printf("ALLOCATING 3 CHUNKS. MIDDLE CHUNK IS HALF THE HEAP SIZE...\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(SIZE_OF_HEAP / 2);
    chunks[2] = my_malloc(CHUNK_SIZE);
    printf("FREEING MIDDLE CHUNK...\n");
    my_free(chunks[1]);
    printf("ALLOCATING 1 CHUNK...\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING CHUNK WAS ALLOCATED IN MIDDLE...\n");
    audit();
    assert(chunks[1] > chunks[0] && chunks[1] < chunks[2]);
    printf("ALLOCATING 1 CHUNK...\n");
    chunks[3] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING CHUNK WAS ALLOCATED IN MIDDLE...\n");
    audit();
    assert(chunks[3] > chunks[0] && chunks[3] < chunks[2]);
    printf("ALLOCATING 1 CHUNK...\n");
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING CHUNK WAS ALLOCATED IN MIDDLE...\n");
    audit();
    assert(chunks[4] > chunks[0] && chunks[4] < chunks[2]);
    printf("ALLOCATING 1 CHUNK...\n");
    chunks[5] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING CHUNK WAS ALLOCATED AT THE END...\n");
    audit();
    assert(chunks[5] > chunks[2]);
    free_all_chunks();
    passed();

    success("ALL WORST FIT ALLOCATION TESTS PASSED");
}

void test_malloc_bad_size()
{
    emphasis("TESTING MALLOC RETURNS NULL ON BAD VALUE SIZE REQUESTS");

    free_all_chunks();
    void *chunks[MAX_CHUNKS];

    printf("REQUESTING 1 CHUNK THAT IS TWICE THE SIZE OF HEAP...\n");
    chunks[0] = my_malloc(2 * SIZE_OF_HEAP);
    printf("VERIFYING RETURN IS NULL...\n");
    audit();
    assert(chunks[0] == NULL);
    passed();

    printf("ALLOCATING 2 CHUNKS WITH SUM SLIGHTLY SMALLER THAN HEAP SIZE...\n");
    chunks[0] = my_malloc((SIZE_OF_HEAP / 2) - (CHUNK_SIZE / 2));
    chunks[1] = my_malloc((SIZE_OF_HEAP / 2) - (CHUNK_SIZE / 2));
    printf("REQUESTING ADDITIONAL CHUNK WITH SIZE GREATER THAN REMAINING HEAP SIZE...\n");
    chunks[2] = my_malloc(CHUNK_SIZE);
    printf("VERIFYING RETURN IS NULL...\n");
    audit();
    assert(chunks[2] == NULL);
    free_all_chunks();
    passed();

    printf("REQUESTING 1 CHUNK OF SIZE -1...\n");
    chunks[0] = my_malloc(-1);
    printf("VERIFYING RETURN IS NULL...\n");
    audit();
    assert(chunks[0] == NULL);
    passed();

    printf("REQUESTING LARGE NEGATIVE SIZE CHUNK...\n");
    chunks[0] = my_malloc(-SIZE_OF_HEAP / 2);
    printf("VERIFYING RETURN IS NULL...\n");
    audit();
    assert(chunks[0] == NULL);
    passed();

    printf("REQUESTING SIZE 0 CHUNK...\n");
    chunks[0] = my_malloc(0);
    printf("VERIFYING RETURN IS NULL...\n");
    audit();
    assert(chunks[0] == NULL);
    passed();

    success("ALL MALLOC BAD SIZE TESTS PASSED");
}

void test_all()
{
    emphasis("RUNNING ALL TESTS");
    test_free_chunk_reuse();
    test_sorted_free_list();
    test_splitting_free_chunks();
    test_coalesce();
    test_alternating_sequence();
    test_worst_fit();
    test_malloc_bad_size();
    success("ALL TESTS PASSED");
}

#pragma endregion Tests

/* Sets standard chunk size for tests. */
void init_tests()
{
    MAX_CHUNKS = 10;
    CHUNK_SIZE = SIZE_OF_HEAP / 20;

    printf("Tests using standard chunk size of %zu\n", CHUNK_SIZE);
    printf("This translate to a total aligned size of %zu\n", align(CHUNK_SIZE));
}