#include <stdlib.h>
#include <stddef.h>
#include <sys/reent.h>

void* _malloc_r(struct _reent* unused, size_t size)
{
    (void) unused;
    return malloc(size);
}

void _free_r(struct _reent* unused, void* ptr)
{
    (void) unused;
    free(ptr);
}

void* _realloc_r(struct _reent* unused, void* ptr, size_t size)
{
    (void) unused;
    return realloc(ptr, size);
}

void* _calloc_r(struct _reent* unused, size_t count, size_t size)
{
    (void) unused;
    return calloc(count, size);
}

