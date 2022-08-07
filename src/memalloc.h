#pragma once
#include <stddef.h>

void* memalloc(size_t size);
void memfree(void* ptr);
void memdump();