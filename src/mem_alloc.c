#include "mem_alloc.h"

static unsigned char* mem = NULL;
static unsigned char* mem_mask = NULL;
static size_t mem_size = 0;

static int is_busy(size_t index)
{
	size_t offset;
	size_t byte_num;

	if (index > mem_size)
	{
		return 1;
	}

	offset = index / 8;
	byte_num = index % 8;

	if (((mem_mask[offset] & (1 << byte_num)) & 0xFF) == 0)
	{
		return 0;
	}

	return 1;
}

static int is_area_busy(size_t start, size_t size)
{
	size_t i = 0;
	for (i = 0; i < size; i++)
	{
		if (is_busy(start + size))
		{
			return 1;
		}
	}

	return 0;
}

static void mark(size_t index, char state)
{
	size_t offset;
	size_t byte_num;

	if (index >= mem_size)
	{
		return;
	}

	offset = index / 8;
	byte_num = index % 8;

	if (state == 0)
	{
		mem_mask[offset] &= (~(1 << byte_num)) & 0xFF;
	}
	else
	{
		mem_mask[offset] |= 1 << byte_num;
	}
}

/* cppcheck-suppress unusedFunction */
void mem_init(void* buffer, size_t size)
{
	size_t i = 0;
	mem = buffer;
	for (i = 0; i < size; i++)
	{
		mem[i] = 0;
	}

	mem_size = size * 8 / 9;
	mem_mask = mem + mem_size;
}

/* cppcheck-suppress unusedFunction */
void* mem_alloc(size_t size)
{
	size_t i = 0;
	size_t j = 0;

	for (i = 0; i < mem_size; i++)
	{
		if ((size_t)(mem + i) % MEM_ALLOC_ALIGN != 0)
		{
			continue;
		}

		if (is_busy(i))
		{
			continue;
		}

		if (i != 0 && is_busy(i - 1))
		{
			continue;
		}

		if (is_area_busy(i, size))
		{
			continue;
		}

		for (j = 0; j < size; j++)
		{
			mark(i + j, 1);
		}

		return mem + i;
	}

	return NULL;
}

/* cppcheck-suppress unusedFunction */
void mem_free(void* ptr)
{
	size_t i = (unsigned char*)ptr - mem;

	for (; i < mem_size; i++)
	{
		if (is_busy(i))
		{
			mark(i, 0);
		}
		else
		{
			return;
		}
	}
}
