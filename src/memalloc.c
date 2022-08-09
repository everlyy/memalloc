#include "memalloc.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define MAP_TYPE uint32_t
#define MAP_TYPE_SIZE (sizeof(MAP_TYPE) * 8)
#define AREA_INFO_SIZE sizeof(size_t)

#define MEM_SIZE 8192
#define CHUNK_SIZE 16
#define CHUNK_COUNT MEM_SIZE / CHUNK_SIZE
#define MAP_SIZE MEM_SIZE / CHUNK_SIZE / MAP_TYPE_SIZE

#define COL_RESET "\033[0m"
#define COL_USED "\033[40m"
#define COL_FREE "\033[107m"

uint8_t memory[MEM_SIZE];
MAP_TYPE memmap[MAP_SIZE];

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

	// Add sizeof(size_t) so we can store the amount of chunks as the first 4 bytes
	size += sizeof(size_t);

	size_t chunks = 0;
	int fit = find_fit(size, &chunks);
	if(fit < 0)
		return NULL;

	set_size_in_map(fit, chunks, 1);

	void* allocated_area = (void*)((uintptr_t)memory + (uintptr_t)fit);

	// Store the amount of chunks allocated in the allocated area
	*((size_t*)allocated_area) = chunks;

	total_used_chunks += chunks;
	return (void*)((uintptr_t)allocated_area + (uintptr_t)AREA_INFO_SIZE);
}

void memfree(void* ptr) {
	free_call_count++;

	size_t idx = (size_t)((uintptr_t)ptr - (uintptr_t)memory - (uintptr_t)AREA_INFO_SIZE);
	size_t chunk_count = (size_t)memory[idx];

	if(idx < 0 || idx > CHUNK_COUNT)
		return;
	total_free_chunks += chunk_count;
	set_size_in_map(idx, chunk_count, 0);
}

/* just don't look at the next functions these just look so terrible */
void memarea_info(void* ptr) {
	size_t idx = (size_t)((uintptr_t)ptr - (uintptr_t)memory - (uintptr_t)AREA_INFO_SIZE);
	size_t chunk_count = (size_t)memory[idx];

	printf(COL_USED"        "COL_FREE);
	printf("                         "COL_RESET"\n");
	printf("|       |\n");
	printf("|       +- usable area (%ld bytes)\n", chunk_count * CHUNK_SIZE - AREA_INFO_SIZE);
	printf("+- area info (%ld bytes)\n", AREA_INFO_SIZE);
}

void memdump() {
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
	printf("memory overview: "COL_USED" "COL_RESET" used; "COL_FREE" "COL_RESET" free\n");
	printf("chunk");
	for(size_t i = 0; i < MAP_SIZE * MAP_TYPE_SIZE; i++) {
		if(i % MAP_TYPE_SIZE == 0)
			printf("\n%ld\t", i);
		int set = get_bit(i);
		printf("%s "COL_RESET, set ? COL_USED : COL_FREE);
	}
	printf("\n");
}