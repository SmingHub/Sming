#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

#include "unity_fixture.h"

#include "umm_malloc.h"
#include "umm_malloc_cfg.h"

/* Use the default DBGLOG_LEVEL and DBGLOG_FUNCTION */

#define DBGLOG_LEVEL 0

#include "dbglog/dbglog.h"

char test_umm_heap[UMM_MALLOC_CFG_HEAP_SIZE];

// Note, the block size calculation depends on knowledge of the internals
// of umm_malloc.c which are not to be exposed to the user of the library

#define UMM_BLOCK_SIZE (8)
#define UMM_LASTBLOCK ((UMM_MALLOC_CFG_HEAP_SIZE-UMM_BLOCK_SIZE)/UMM_BLOCK_SIZE)

bool check_all_bytes ( uint8_t * p, size_t s, uint8_t v) {
	while ( (*p == v) && s ) {
		++p;
	 	--s;
        }

	return ( s == 0 );
}

bool get_block_is_free ( int  b ) {
    return ( (((uint16_t *)((void *)(&(((uint64_t *)test_umm_heap)[b]))))[0] & 0x8000) == 0x8000 );
}

uint16_t get_block_next ( int b ) {
    return ( ((uint16_t *)((void *)(&(((uint64_t *)test_umm_heap)[b]))))[0] & 0x7FFF );
}

uint16_t get_block_prev ( int b ) {
    return ( ((uint16_t *)((void *)(&(((uint64_t *)test_umm_heap)[b]))))[1] );
}

uint16_t get_block_next_free ( int b ) {
    return ( ((uint16_t *)((void *)(&(((uint64_t *)test_umm_heap)[b]))))[2] );
}

uint16_t get_block_prev_free ( int b ) {
    return ( ((uint16_t *)((void *)(&(((uint64_t *)test_umm_heap)[b]))))[3] );
}

struct block_test_values {
    uint16_t block;
    bool     is_free;
    uint16_t next;
    uint16_t prev;
    uint16_t next_free;
    uint16_t prev_free;
};

# define ARRAYELEMENTCOUNT(x) (sizeof (x) / sizeof (x)[0])

#define TEST_MSG_LEN (132)
char block_test_msg[TEST_MSG_LEN]; 
char block_actual_msg[TEST_MSG_LEN]; 
char test_msg[256];

bool check_block (struct block_test_values *t)
{
        snprintf (block_test_msg,   TEST_MSG_LEN, "\nTest__: Block %04d f %d n %04d p %04d nf %04d pf %04d", t->block
                                                                                                           , t->is_free
                                                                                                           , t->next
                                                                                                           , t->prev
                                                                                                           , t->next_free
                                                                                                           , t->prev_free);
        snprintf (block_actual_msg, TEST_MSG_LEN, "\nActual: Block %04d f %d n %04d p %04d nf %04d pf %04d\n", t->block
                                                                                                           , get_block_is_free (t->block)
                                                                                                           , get_block_next(t->block)
                                                                                                           , get_block_prev(t->block)
                                                                                                           , get_block_next_free(t->block)
                                                                                                           , get_block_prev_free(t->block));
        strncpy( test_msg, block_test_msg, 256 );
        strncat( test_msg, block_actual_msg, 256 );

        TEST_ASSERT_EQUAL_MESSAGE ( t->is_free,   get_block_is_free(   t->block ), test_msg );
        TEST_ASSERT_EQUAL_MESSAGE ( t->next,      get_block_next(      t->block ), test_msg );
        TEST_ASSERT_EQUAL_MESSAGE ( t->prev,      get_block_prev(      t->block ), test_msg );
        TEST_ASSERT_EQUAL_MESSAGE ( t->next_free, get_block_next_free( t->block ), test_msg );
        TEST_ASSERT_EQUAL_MESSAGE ( t->prev_free, get_block_prev_free( t->block ), test_msg );

        return true;
}

bool check_blocks (struct block_test_values *t, size_t n)
{
    int i;
    for (i=0; i<n; ++i) {
        TEST_ASSERT_TRUE (check_block (&t[i]));
    }
    return (true);
}

TEST_GROUP(Heap);

TEST_SETUP(Heap)
{
    umm_init ();
}

TEST_TEAR_DOWN(Heap)
{
}

struct block_test_values Initialization_test_values[] =
    { {0            , false, 1            , 0, 1, 1}
    , {1            , true,  UMM_LASTBLOCK, 0, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 1, 0, 0}
    };

TEST(Heap, Initialization)
{
    TEST_ASSERT_TRUE (check_blocks (Initialization_test_values, ARRAYELEMENTCOUNT(Initialization_test_values)));
}

TEST( Heap, FirstMalloc0Bytes )
{
    TEST_ASSERT_EQUAL_PTR ((void *)NULL, (umm_malloc (0)));
    TEST_ASSERT_TRUE (check_blocks (Initialization_test_values, ARRAYELEMENTCOUNT(Initialization_test_values)));
}

struct block_test_values MallocMin_test_values[] =
    { {0            , false, 1            , 0, 2, 2}
    , {1            , false, 2            , 0, 0, 0}
    , {2            , true,  UMM_LASTBLOCK, 1, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 2, 0, 0}
    };
    
TEST(Heap, FirstMalloc1Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (1)));
    TEST_ASSERT_TRUE (check_blocks (MallocMin_test_values, ARRAYELEMENTCOUNT(MallocMin_test_values)));
}

TEST(Heap, FirstMalloc2Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (2)));
    TEST_ASSERT_TRUE (check_blocks (MallocMin_test_values, ARRAYELEMENTCOUNT(MallocMin_test_values)));
}

TEST(Heap, FirstMalloc3Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (3)));
    TEST_ASSERT_TRUE (check_blocks (MallocMin_test_values, ARRAYELEMENTCOUNT(MallocMin_test_values)));
}

TEST(Heap, FirstMalloc4Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (4)));
    TEST_ASSERT_TRUE (check_blocks (MallocMin_test_values, ARRAYELEMENTCOUNT(MallocMin_test_values)));
}

struct block_test_values MallocSmall_test_values[] =
    { {0            , false, 1            , 0, 3, 3}
    , {1            , false, 3            , 0, 0, 0}
    , {3            , true,  UMM_LASTBLOCK, 1, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 3, 0, 0}
    };

TEST(Heap, FirstMalloc5Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (5)));
    TEST_ASSERT_TRUE (check_blocks (MallocSmall_test_values, ARRAYELEMENTCOUNT(MallocSmall_test_values)));
}

TEST(Heap, FirstMalloc12Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (12)));
    TEST_ASSERT_TRUE (check_blocks (MallocSmall_test_values, ARRAYELEMENTCOUNT(MallocSmall_test_values)));
}

struct block_test_values MallocSmallPlus_test_values[] =
    { {0            , false, 1            , 0, 4, 4}
    , {1            , false, 4            , 0, 0, 0}
    , {4            , true,  UMM_LASTBLOCK, 1, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 4, 0, 0}
    };

TEST(Heap, FirstMalloc13Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (13)));
    TEST_ASSERT_TRUE (check_blocks (MallocSmallPlus_test_values, ARRAYELEMENTCOUNT(MallocSmallPlus_test_values)));
}

struct block_test_values MallocMid_test_values[] =
    { {0            , false, 1            , 0, 502, 502}
    , {1            , false, 502          , 0,   0,   0}
    , {502          , true,  UMM_LASTBLOCK, 1,   0,   0}
    , {UMM_LASTBLOCK, false, 0            , 502, 0,   0}
    };

TEST(Heap, FirstMalloc4000Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (4000)));
    TEST_ASSERT_TRUE (check_blocks (MallocMid_test_values, ARRAYELEMENTCOUNT(MallocMid_test_values)));
}

struct block_test_values MallocLarge_test_values[] =
    { {0             , false, 1            , 0, 0, 0}
    , {1             , false, UMM_LASTBLOCK, 0, 0, 0}
    , {UMM_LASTBLOCK , false, 0            , 1, 0, 0}
    };

TEST(Heap, FirstMalloc65516Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], (umm_malloc (65516)));
    TEST_ASSERT_TRUE (check_blocks (MallocLarge_test_values, ARRAYELEMENTCOUNT(MallocLarge_test_values)));
}

TEST(Heap, FirstMalloc65517Bytes)
{
    TEST_ASSERT_EQUAL_PTR((void *)NULL, (umm_malloc (65517)));
}


TEST_GROUP_RUNNER(FirstMalloc)
{
    RUN_TEST_CASE(Heap, Initialization);
    RUN_TEST_CASE(Heap, FirstMalloc0Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc1Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc2Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc3Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc4Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc5Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc12Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc13Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc4000Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc65516Bytes);
    RUN_TEST_CASE(Heap, FirstMalloc65517Bytes);
}

TEST_GROUP(MultiMalloc);

TEST_SETUP(MultiMalloc)
{
    umm_init ();
}

TEST_TEAR_DOWN(MultiMalloc)
{
}

struct block_test_values MultiMallocManySmall_test_values[] =
    { {0            , false, 1            , 0,  6,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , false, 3            , 1,  0,  0}
    , {3            , false, 4            , 2,  0,  0}
    , {4            , false, 5            , 3,  0,  0}
    , {5            , false, 6            , 4,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  0}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(MultiMalloc, ManySmall)
{
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 +  0], umm_malloc(4));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 +  8], umm_malloc(4));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 16], umm_malloc(4));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 24], umm_malloc(4));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 32], umm_malloc(4));

    TEST_ASSERT_TRUE (check_blocks (MultiMallocManySmall_test_values, ARRAYELEMENTCOUNT(MultiMallocManySmall_test_values)));
}

struct block_test_values MultiMallocManyMed_test_values[] =
    { {   0         , false,    1         ,    0 ,  2501,  2501}
    , {   1         , false,  501         ,    0 ,  0,  0}
    , { 501         , false, 1001         ,    1 ,  0,  0}
    , {1001         , false, 1501         ,  501 ,  0,  0}
    , {1501         , false, 2001         , 1001 ,  0,  0}
    , {2001         , false, 2501         , 1501 ,  0,  0}
    , {2501         , true,  UMM_LASTBLOCK, 2001 ,  0,  0}
    , {UMM_LASTBLOCK, false, 0            , 2501 ,  0,  0}
    };

TEST(MultiMalloc, ManyMed)
{
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 +     0], umm_malloc(3996));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 +  4000], umm_malloc(3996));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 +  8000], umm_malloc(3996));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 12000], umm_malloc(3996));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 16000], umm_malloc(3996));

    TEST_ASSERT_TRUE (check_blocks (MultiMallocManyMed_test_values, ARRAYELEMENTCOUNT(MultiMallocManyMed_test_values)));
}

struct block_test_values MultiMallocManyLarge_test_values[] =
    { {    0        , false, 1            ,    0 ,  7501,  7501}
    , {    1        , false, 2501         ,    0 ,     0,     0}
    , { 2501        , false, 5001         ,    1 ,     0,     0}
    , { 5001        , false, 7501         , 2501 ,     0,     0}
    , { 7501        , true,  UMM_LASTBLOCK, 5001 ,     0,     0}
    , {UMM_LASTBLOCK, false, 0            , 7501 ,     0,     0}
    };

TEST(MultiMalloc, ManyLarge)
{
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 +     0], umm_malloc(19996));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 20000], umm_malloc(19996));
    TEST_ASSERT_EQUAL_PTR ((void *)&test_umm_heap[12 + 40000], umm_malloc(19996));
    TEST_ASSERT_EQUAL_PTR ((void *)NULL                      , umm_malloc(19996));

    TEST_ASSERT_TRUE (check_blocks (MultiMallocManyLarge_test_values, ARRAYELEMENTCOUNT(MultiMallocManyLarge_test_values)));
}

TEST_GROUP_RUNNER(MultiMalloc)
{
    RUN_TEST_CASE(MultiMalloc, ManySmall);
    RUN_TEST_CASE(MultiMalloc, ManyMed);
    RUN_TEST_CASE(MultiMalloc, ManyLarge);
}

void *p[5];

TEST_GROUP(Free);

TEST_SETUP(Free)
{
    umm_init ();

    p[0] = umm_malloc (4);
    p[1] = umm_malloc (4);
    p[2] = umm_malloc (4);
    p[3] = umm_malloc (4);
    p[4] = umm_malloc (4);
}

TEST_TEAR_DOWN(Free)
{
}

TEST(Free, NullPtr)
{
    umm_free((void *)NULL);
}

TEST(Free, BadPtr)
{
    TEST_PASS();

    umm_free((void *)1);
}

struct block_test_values FreeFirst_test_values[] =
    { {0            , false, 1            , 0, 1, 1}
    , {1            , true,  UMM_LASTBLOCK, 0, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 1, 0, 0}
    };

TEST(Free, First)
{
    umm_init ();
    umm_free (umm_malloc(4));
    TEST_ASSERT_TRUE (check_blocks (FreeFirst_test_values, ARRAYELEMENTCOUNT(FreeFirst_test_values)));
}

struct block_test_values FreeLast_test_values[] =
    { {0            , false, 1            , 0,  5,  5}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , false, 3            , 1,  0,  0}
    , {3            , false, 4            , 2,  0,  0}
    , {4            , false, 5            , 3,  0,  0}
    , {5            , true,  UMM_LASTBLOCK, 4,  0,  0}
    , {UMM_LASTBLOCK, false, 0            , 5,  0,  0}
    };

TEST(Free, Last)
{
    umm_free (p[4]);

    TEST_ASSERT_TRUE (check_blocks (FreeLast_test_values, ARRAYELEMENTCOUNT(FreeLast_test_values)));
}

struct block_test_values FreeSecondLast_test_values[] =
    { {0            , false, 1            , 0,  4,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , false, 3            , 1,  0,  0}
    , {3            , false, 4            , 2,  0,  0}
    , {4            , true,  5            , 3,  6,  0}
    , {5            , false, 6            , 4,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  4}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, SecondLast)
{
    umm_free (p[3]);

    TEST_ASSERT_TRUE (check_blocks (FreeSecondLast_test_values, ARRAYELEMENTCOUNT(FreeSecondLast_test_values)));
}

struct block_test_values FreeAssimilateUp_test_values[] =
    { {0            , false, 1            , 0,  3,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , false, 3            , 1,  0,  0}
    , {3            , true,  5            , 2,  6,  0}
    , {5            , false, 6            , 3,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  3}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, AssimilateUp)
{
    umm_free (p[3]);
    umm_free (p[2]);

    TEST_ASSERT_TRUE (check_blocks (FreeAssimilateUp_test_values, ARRAYELEMENTCOUNT(FreeAssimilateUp_test_values)));
}

struct block_test_values FreeAssimilateDown_test_values[] =
    { {0            , false, 1            , 0,  3,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , false, 3            , 1,  0,  0}
    , {3            , true,  5            , 2,  6,  0}
    , {5            , false, 6            , 3,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  3}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, AssimilateDown)
{
    umm_free (p[2]);
    umm_free (p[3]);

    TEST_ASSERT_TRUE (check_blocks (FreeAssimilateDown_test_values, ARRAYELEMENTCOUNT(FreeAssimilateDown_test_values)));
}

struct block_test_values FreeAssimilateUpDown_test_values[] =
    { {0            , false, 1            , 0,  2,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , true,  5            , 1,  6,  0}
    , {5            , false, 6            , 2,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  2}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, AssimilateUpDown)
{
    umm_free (p[3]);
    umm_free (p[1]);
    umm_free (p[2]);

    TEST_ASSERT_TRUE (check_blocks (FreeAssimilateUpDown_test_values, ARRAYELEMENTCOUNT(FreeAssimilateUpDown_test_values)));
}

struct block_test_values FreeAssimilateDownUp_test_values[] =
    { {0            , false, 1            , 0,  2,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , true,  5            , 1,  6,  0}
    , {5            , false, 6            , 2,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  2}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, AssimilateDownUp)
{
    umm_free (p[2]);
    umm_free (p[1]);
    umm_free (p[3]);

    TEST_ASSERT_TRUE (check_blocks (FreeAssimilateDownUp_test_values, ARRAYELEMENTCOUNT(FreeAssimilateDownUp_test_values)));
}

struct block_test_values FreeAssimilateFirst_test_values[] =
    { {0            , false, 1            , 0,  1,  6}
    , {1            , true,  3            , 0,  6,  0}
    , {3            , false, 4            , 1,  0,  0}
    , {4            , false, 5            , 3,  0,  0}
    , {5            , false, 6            , 4,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  1}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, AssimilateFirst)
{
    umm_free (p[1]);
    umm_free (p[0]);

    TEST_ASSERT_TRUE (check_blocks (FreeAssimilateFirst_test_values, ARRAYELEMENTCOUNT(FreeAssimilateFirst_test_values)));
}

struct block_test_values FreeAssimilateLast_test_values[] =
    { {0            , false, 1            , 0,  4,  4}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , false, 3            , 1,  0,  0}
    , {3            , false, 4            , 2,  0,  0}
    , {4            , true,  UMM_LASTBLOCK, 3,  0,  0}
    , {UMM_LASTBLOCK, false, 0            , 4,  0,  0}
    };

TEST(Free, AssimilateLast)
{
    umm_free (p[3]);
    umm_free (p[4]);

    TEST_ASSERT_TRUE (check_blocks (FreeAssimilateLast_test_values, ARRAYELEMENTCOUNT(FreeAssimilateLast_test_values)));
}


struct block_test_values FreeHiLo_test_values[] =
    { {0            , false, 1            , 0,  2,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , true,  3            , 1,  4,  0}
    , {3            , false, 4            , 2,  0,  0}
    , {4            , true,  5            , 3,  6,  2}
    , {5            , false, 6            , 4,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  4}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, HiLo)
{
    umm_free (p[3]);
    umm_free (p[1]);

    TEST_ASSERT_TRUE (check_blocks (FreeHiLo_test_values, ARRAYELEMENTCOUNT(FreeHiLo_test_values)));
}

struct block_test_values FreeLoHi_test_values[] =
    { {0            , false, 1            , 0,  4,  6}
    , {1            , false, 2            , 0,  0,  0}
    , {2            , true,  3            , 1,  6,  4}
    , {3            , false, 4            , 2,  0,  0}
    , {4            , true,  5            , 3,  2,  0}
    , {5            , false, 6            , 4,  0,  0}
    , {6            , true,  UMM_LASTBLOCK, 5,  0,  2}
    , {UMM_LASTBLOCK, false, 0            , 6,  0,  0}
    };

TEST(Free, LoHi)
{
    umm_free (p[1]);
    umm_free (p[3]);

    TEST_ASSERT_TRUE (check_blocks (FreeLoHi_test_values, ARRAYELEMENTCOUNT(FreeLoHi_test_values)));
}

TEST_GROUP_RUNNER(Free)
{
    RUN_TEST_CASE(Free, NullPtr);
    RUN_TEST_CASE(Free, BadPtr);
    RUN_TEST_CASE(Free, First);
    RUN_TEST_CASE(Free, Last);
    RUN_TEST_CASE(Free, SecondLast);
    RUN_TEST_CASE(Free, AssimilateUp);
    RUN_TEST_CASE(Free, AssimilateDown);
    RUN_TEST_CASE(Free, AssimilateUpDown);
    RUN_TEST_CASE(Free, AssimilateDownUp);
    RUN_TEST_CASE(Free, AssimilateFirst);
    RUN_TEST_CASE(Free, AssimilateLast);
    RUN_TEST_CASE(Free, HiLo);
    RUN_TEST_CASE(Free, LoHi);
}

TEST_GROUP(Realloc);

TEST_SETUP(Realloc)
{
    umm_init ();
}

TEST_TEAR_DOWN(Realloc)
{
}

struct block_test_values ReallocTooBig_test_values[] =
    { {0            , false, 1            , 0, 2, 2}
    , {1            , false, 2            , 0, 0, 0}
    , {2            , true,  UMM_LASTBLOCK, 1, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 2, 0, 0}
    };

TEST(Realloc, TooBig)
{
    void *foo = umm_malloc (4);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], foo                        );
    TEST_ASSERT_TRUE (check_blocks (ReallocTooBig_test_values, ARRAYELEMENTCOUNT(ReallocTooBig_test_values)));

    // Realloc with a request that is too big should return NULL and leave the original memory untouched.
 
    TEST_ASSERT_EQUAL_PTR((void *)NULL, (umm_realloc (foo,UMM_MALLOC_CFG_HEAP_SIZE*2)));
    TEST_ASSERT_TRUE (check_blocks (ReallocTooBig_test_values, ARRAYELEMENTCOUNT(ReallocTooBig_test_values)));
}

struct block_test_values ReallocSameSize_test_values[] =
    { {0            , false, 1            , 0, 2, 2}
    , {1            , false, 2            , 0, 0, 0}
    , {2            , true,  UMM_LASTBLOCK, 1, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 2, 0, 0}
    };

TEST(Realloc, SameSize)
{
    void *foo = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], foo                        );
    TEST_ASSERT_TRUE (check_blocks (ReallocSameSize_test_values, ARRAYELEMENTCOUNT(ReallocSameSize_test_values)));

    // Realloc with a request that is same size or block size should leave the original memory untouched.
 
    TEST_ASSERT_EQUAL_PTR((void *)foo, (umm_realloc (foo, 2)));
    TEST_ASSERT_TRUE (check_blocks (ReallocSameSize_test_values, ARRAYELEMENTCOUNT(ReallocSameSize_test_values)));

    // Realloc with a request that is same size or block size should leave the original memory untouched.
 
    TEST_ASSERT_EQUAL_PTR((void *)foo, (umm_realloc (foo, 1)));
    TEST_ASSERT_TRUE (check_blocks (ReallocSameSize_test_values, ARRAYELEMENTCOUNT(ReallocSameSize_test_values)));
 
    // Realloc with a request that is same size or block size should leave the original memory untouched.
 
    TEST_ASSERT_EQUAL_PTR((void *)foo, (umm_realloc (foo, 4)));
    TEST_ASSERT_TRUE (check_blocks (ReallocSameSize_test_values, ARRAYELEMENTCOUNT(ReallocSameSize_test_values)));
}

struct block_test_values ReallocFree_test_values[] =
    { {0            , false, 1            , 0, 1, 1}
    , {1            , true,  UMM_LASTBLOCK, 0, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 1, 0, 0}
    };

TEST(Realloc, Free)
{
    void *foo = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], foo                        );

    // Realloc with a request that is 0 size should free the block
 
    TEST_ASSERT_EQUAL_PTR((void *)NULL, (umm_realloc (foo, 0)));
    TEST_ASSERT_TRUE (check_blocks (ReallocFree_test_values, ARRAYELEMENTCOUNT(ReallocFree_test_values)));
}

struct block_test_values ReallocFreeRealloc_test_values[] =
    { {0            , false, 1            , 0, 2, 2}
    , {1            , false, 2            , 0, 0, 0}
    , {2            , true,  UMM_LASTBLOCK, 1, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 2, 0, 0}
    };

TEST(Realloc, FreeRealloc)
{
    void *foo = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], foo                        );
    TEST_ASSERT_TRUE (check_blocks (ReallocFreeRealloc_test_values, ARRAYELEMENTCOUNT(ReallocFreeRealloc_test_values)));

    // Realloc with a request that is 0 size should free the block
 
    TEST_ASSERT_EQUAL_PTR((void *)NULL, (umm_realloc (foo, 0)));

    // Realloc with a request that is same size or block size should leave the original memory untouched.
 
    TEST_ASSERT_EQUAL_PTR((void *)foo, (umm_realloc (NULL, 4)));
    TEST_ASSERT_TRUE (check_blocks (ReallocSameSize_test_values, ARRAYELEMENTCOUNT(ReallocSameSize_test_values)));
}
 
struct block_test_values ReallocAssimilateUp[] =
    { {0            , false, 1            , 0, 4, 4}
    , {1            , false, 2            , 0, 0, 0}
    , {2            , false, 4            , 1, 0, 0}
    , {4            , true,  UMM_LASTBLOCK, 2, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 4, 0, 0}
    };

TEST(Realloc, AssimilateUp)
{
    void *mem0 = umm_malloc (2);
    void *mem1 = umm_malloc (2);
    void *mem2 = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], mem0                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[20], mem1                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[28], mem2                        );

    // Free the last block and then realloc the middle block to use it
 
    umm_free( mem2 );

    TEST_ASSERT_EQUAL_PTR((void *)mem1, (umm_realloc (mem1, 5)));
    TEST_ASSERT_TRUE (check_blocks (ReallocAssimilateUp, ARRAYELEMENTCOUNT(ReallocAssimilateUp)));
}

struct block_test_values ReallocAssimilateDown[] =
    { {0            , false, 1            , 0, 4, 4}
    , {1            , false, 3            , 0, 0, 0}
    , {3            , false, 4            , 1, 0, 0}
    , {4            , true,  UMM_LASTBLOCK, 3, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 4, 0, 0}
    };

TEST(Realloc, AssimilateDown)
{
    void *mem0 = umm_malloc (2);
    void *mem1 = umm_malloc (2);
    void *mem2 = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], mem0                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[20], mem1                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[28], mem2                        );

    // Free the last block and then realloc the middle block to use it
 
    umm_free( mem0 );

    TEST_ASSERT_EQUAL_PTR((void *)mem0, (umm_realloc (mem1, 5)));
    TEST_ASSERT_TRUE (check_blocks (ReallocAssimilateDown, ARRAYELEMENTCOUNT(ReallocAssimilateDown)));
}

struct block_test_values ReallocAssimilateUpDown[] =
    { {0            , false, 1            , 0, 5, 5}
    , {1            , false, 4            , 0, 0, 0}
    , {4            , false, 5            , 1, 0, 0}
    , {5            , true,  UMM_LASTBLOCK, 4, 0, 0}
    , {UMM_LASTBLOCK, false, 0            , 5, 0, 0}
    };

TEST(Realloc, AssimilateUpDown)
{
    void *mem0 = umm_malloc (2);
    void *mem1 = umm_malloc (2);
    void *mem2 = umm_malloc (2);
    void *mem3 = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], mem0                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[20], mem1                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[28], mem2                        );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[36], mem3                        );

    // Free the last block and then realloc the middle block to use it
 
    umm_free( mem0 );
    umm_free( mem2 );

    TEST_ASSERT_EQUAL_PTR((void *)mem0, (umm_realloc (mem1, 20)));
    TEST_ASSERT_TRUE (check_blocks (ReallocAssimilateUpDown, ARRAYELEMENTCOUNT(ReallocAssimilateUpDown)));
}

struct block_test_values ReallocNewBlock[] =
    { {0            , false, 1            , 0, 2, 6}
    , {1            , false, 2            , 0, 0, 0}
    , {2            , true , 3            , 1, 6, 0}
    , {3            , false, 4            , 2, 0, 0}
    , {4            , false, 6            , 3, 0, 0}
    , {6            , true,  UMM_LASTBLOCK, 4, 0, 2}
    , {UMM_LASTBLOCK, false, 0            , 6, 0, 0}
    };

TEST(Realloc, NewBlock)
{
    void *mem0 = umm_malloc (2);
    void *mem1 = umm_malloc (2);
    void *mem2 = umm_malloc (2);

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[12], mem0 );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[20], mem1 );
    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[28], mem2 );

    /* Realloc the middle block - should need a totally new block */

    TEST_ASSERT_EQUAL_PTR((void *)&test_umm_heap[36], (umm_realloc(mem1, 5)));
    TEST_ASSERT_TRUE (check_blocks (ReallocNewBlock, ARRAYELEMENTCOUNT(ReallocNewBlock)));
}

TEST_GROUP_RUNNER(Realloc)
{
    RUN_TEST_CASE(Realloc, TooBig);
    RUN_TEST_CASE(Realloc, SameSize);
    RUN_TEST_CASE(Realloc, Free);
    RUN_TEST_CASE(Realloc, FreeRealloc);
    RUN_TEST_CASE(Realloc, AssimilateUp);
    RUN_TEST_CASE(Realloc, AssimilateDown);
    RUN_TEST_CASE(Realloc, AssimilateUpDown);
    RUN_TEST_CASE(Realloc, NewBlock);
}



TEST_GROUP(Poison);

TEST_SETUP(Poison)
{
    umm_init ();
}

TEST_TEAR_DOWN(Poison)
{
}

TEST(Poison, First)
{
    TEST_ASSERT_NOT_NULL (umm_poison_malloc(4));
}

TEST(Poison, ClobberLeading)
{
    void *p = umm_poison_malloc(64);
    
    p = p - 1;
    *(char *)p = 0x00;

    TEST_ASSERT_EQUAL(0, POISON_CHECK());
}

TEST(Poison, ClobberTrailing)
{
    void *p = umm_poison_malloc(64);
    
    p = p + 64;
    *(char *)p = 0x00;

    TEST_ASSERT_EQUAL(0, POISON_CHECK());
}

TEST(Poison, Random)
{
    void *p[100];
    int i,j;
    size_t s;

    srand(0);

    for (i=0; i<100; ++i)
       p[i] = (void *)NULL;
 
    for (i=0; i<100000; ++i) {

        TEST_ASSERT_NOT_EQUAL(0, INTEGRITY_CHECK());
        TEST_ASSERT_NOT_EQUAL(0, POISON_CHECK());

        j = rand()%100;
        s = rand()%64;

        if (p[j]) {
            umm_poison_free(p[j]);
        }

        p[j] = umm_poison_malloc(s);

        if (0==s) {
            TEST_ASSERT_NULL( p[j] );
        } else {
            TEST_ASSERT_NOT_NULL( p[j] );
        }
    }
}

TEST(Poison, Stress)
{
  void *p[256];
  int i,j;
  size_t s;

  srand(0);

  for( j=0; j<256; ++j )
    p[j] = (void *)NULL;

  for( i=0; i<100000; ++i ) {
    j = rand()%256;

    switch( rand() % 16 ) {

      case  0:
      case  1:
      case  2:
      case  3:
      case  4:
      case  5:
      case  6:
        {
          p[j] = umm_poison_realloc(p[j], 0);
          TEST_ASSERT_NULL( p[j] );
          break;
        }
      case  7:
      case  8:
        {
          s = rand()%64;
          p[j] = umm_poison_realloc(p[j], s );
          if( s ) {
              TEST_ASSERT_NOT_NULL( p[j] );
              memset(p[j], 0xfe, s);
          } else {
              TEST_ASSERT_NULL( p[j] );
          }
          break;
        }

      case  9:
      case 10:
      case 11:
      case 12:
        {
          s = rand()%100;
          p[j] = umm_poison_realloc(p[j], s );

          if( s ) {
              TEST_ASSERT_NOT_NULL( p[j] );
              memset(p[j], 0xfe, s);
          } else {
              TEST_ASSERT_NULL( p[j] );
          }
          break;
        }

      case 13:
      case 14:
        {
          s = rand()%200;
          umm_poison_free(p[j]);
          p[j] = umm_poison_calloc( 1, s );

          if( s ) {
              TEST_ASSERT_NOT_NULL( p[j] );
              TEST_ASSERT_TRUE( check_all_bytes(p[j], s, 0x00) );
              memset(p[j], 0xfe, s);
          } else {
              TEST_ASSERT_NULL( p[j] );
          } 
          break;
        }

      default:
        {
          s = rand()%400;
          umm_poison_free(p[j]);
          p[j] = umm_poison_malloc( s );

          if( s ) {
              TEST_ASSERT_NOT_NULL( p[j] );
              memset(p[j], 0xfe, s);
          } else {
              TEST_ASSERT_NULL( p[j] );
          }
          break;
        }
    }

    TEST_ASSERT_NOT_EQUAL(0, INTEGRITY_CHECK());
    TEST_ASSERT_NOT_EQUAL(0, POISON_CHECK());
  }

  umm_info( 0, true );
  DBGLOG_FORCE( true, "Free Heap Size: %ld\n", umm_free_heap_size() );
}

TEST_GROUP_RUNNER(Poison)
{
    RUN_TEST_CASE(Poison, First);
    RUN_TEST_CASE(Poison, ClobberLeading);
    RUN_TEST_CASE(Poison, ClobberTrailing);
    RUN_TEST_CASE(Poison, Random);
    RUN_TEST_CASE(Poison, Stress);
}

static void runAllTests (void)
{
    RUN_TEST_GROUP(FirstMalloc);
    RUN_TEST_GROUP(MultiMalloc);
    RUN_TEST_GROUP(Free);
    RUN_TEST_GROUP(Poison);
    RUN_TEST_GROUP(Realloc);
}

int main (int argc, const char* argv[])
{
	return UnityMain (argc, argv, runAllTests);
}
