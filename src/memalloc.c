#include "memalloc.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define MAP_TYPE uint32_t
#define MAP_TYPE_SIZE (sizeof(MAP_TYPE) * 8)

#define MEM_SIZE 8192
#define CHUNK_SIZE 16
#define CHUNK_COUNT MEM_SIZE / CHUNK_SIZE
#define MAP_SIZE MEM_SIZE / CHUNK_SIZE / MAP_TYPE_SIZE

uint8_t memory[MEM_SIZE];
MAP_TYPE memmap[MAP_SIZE];
size_t sizemap[CHUNK_COUNT];

size_t alloc_call_count = 0;
size_t free_call_count = 0;
size_t total_used_chunks = 0;
size_t total_free_chunks = 0;

int get_bit(int bit) {
	int map_idx = bit / MAP_TYPE_SIZE;
	int bit_idx = bit % MAP_TYPE_SIZE;
	return (memmap[map_idx] >> bit_idx) & 1; 
}

void set_bit(int bit, int set) {
	int map_idx = bit / MAP_TYPE_SIZE;
	assert(map_idx < MAP_SIZE);
	int bit_idx = bit % MAP_TYPE_SIZE;

	if(set) {
		memmap[map_idx] |= 1 << bit_idx; 
	} else {
		memmap[map_idx] &= ~(1 << bit_idx);
	}
}

void set_size_in_map(int start_bit, size_t chunks, int set) {
	for(size_t i = start_bit; i < chunks + start_bit; i++) {
		set_bit(i, set);
	}
}

int find_fit(size_t size, size_t* chunks) {
	size_t idx = 0;
	size_t count = 0;
	for(size_t i = 0; i < CHUNK_COUNT; i++) {
		int bit_set = get_bit(i);
		if(bit_set) {
			idx = i + 1;
			count = 0;
		}
		else {
			count++;
			if(count * CHUNK_SIZE >= size) {
				*chunks = count;
				return idx;
			}
		}
	}
	return -1;
}

void* memalloc(size_t size) {
	alloc_call_count++;

	if(size < 1)
		return NULL;

	size_t chunks = 0;
	int fit = find_fit(size, &chunks);
	if(fit < 0)
		return NULL;

	set_size_in_map(fit, chunks, 1);

	sizemap[fit] = chunks;
	total_used_chunks += chunks;
	return (void*)((uintptr_t)memory + (uintptr_t)fit);
}

void memfree(void* ptr) {
	free_call_count++;
	size_t idx = (int)((uintptr_t)ptr - (uintptr_t)memory);
	if(idx < 0 || idx > CHUNK_COUNT)
		return;
	total_free_chunks += sizemap[idx];
	set_size_in_map(idx, sizemap[idx], 0);
}

void memdump() {
	const char* col_used = "\033[107m";
	const char* col_free = "\033[40m";
	const char* col_reset = "\033[0m";

	printf("---memalloc dump---\n");
	printf("size of:\n");
	printf("   heap: %d bytes\n", MEM_SIZE);
	printf("  chunk: %d bytes\n", CHUNK_SIZE);
	printf(" bitmap: %d bits\n\n", MAP_SIZE * 8);
	printf("calls to:\n");
	printf(" alloc: %d\n", alloc_call_count);
	printf("  free: %d\n\n", free_call_count);
	printf("chunks:\n");
	printf(" used: %ld (%ld bytes)\n", total_used_chunks, total_used_chunks * CHUNK_SIZE);
	printf(" free: %ld (%ld bytes)\n\n", total_free_chunks, total_free_chunks * CHUNK_SIZE);
	printf("memory overview: %s %s used; %s %s free\n", col_used, col_reset, col_free, col_reset);
	printf("chunk");
	for(size_t i = 0; i < MAP_SIZE * MAP_TYPE_SIZE; i++) {
		if(i % MAP_TYPE_SIZE == 0)
			printf("\n%ld\t", i);
		int set = get_bit(i);
		printf("%s %s", set ? col_used : col_free, col_reset);
	}
	printf("\n");
}