#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <stddef.h>

#ifndef MEM_ALLOC_ALIGN
#define MEM_ALLOC_ALIGN 8
#endif

void mem_init(void* buffer, size_t size);
void* mem_alloc(size_t size);
void mem_free(void* ptr);

#endif
