#include <stdio.h>
#include <time.h>
#include "memalloc.h"

int main(void) {
	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 500000000;

	srand(time(0));
	while(1) {
		void* ptrs[5];
		for(int i = 0; i < 5; i++) {
			void* ptr = memalloc(rand() % 512 + 1);
			if(!ptr) {
				printf("memalloc() returned NULL (out of memory?)\n");
				return 1;
			}
			ptrs[i] = ptr;
		}
		printf("\033[2J");
		memdump();
		nanosleep(&delay, NULL);
		for(int i = 0; i < 5; i++) {
			if(rand() % 2 == 0) {
				memfree(ptrs[i]);
			}
		}
		printf("\033[2J");
		memdump();
		nanosleep(&delay, NULL);
	}
	return 0;
} 