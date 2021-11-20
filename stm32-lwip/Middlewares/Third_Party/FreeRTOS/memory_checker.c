#include "memory_checker.h"
#include <stddef.h>
#include <stdint.h>

struct pointers_t{
	void * loc;
	size_t size;
};

static volatile struct pointers_t _pointers[MAX_PTR_CHECKS] = { 0 };

int multiple_allocations = 0;
int free_without_malloc = 0;
int number_of_allocations = 0;
int total_allocations = 0;

static void update_allocations(void){
	int allocs = 0;
	for (uint32_t i = 0; i < MAX_PTR_CHECKS; i++) {
		if (_pointers[i].loc != NULL) {
			allocs++;

		}
	}

	number_of_allocations = allocs;
}

void add_alloc(void *p, size_t size) {
	bool found = false;
	for (uint32_t i = 0; i < MAX_PTR_CHECKS; i++) {
		if (_pointers[i].loc == p) {
			found = true;
			break;
		}
	}

	if (!found) {
		for (uint32_t i = 0; i < MAX_PTR_CHECKS; i++) {
			if (_pointers[i].loc == NULL) {
				_pointers[i].loc = p;
				_pointers[i].size = size;
				break;
			}
		}
	} else {
		multiple_allocations++;
	}

	update_allocations();
	total_allocations++;
}

void remove_alloc(void *p) {
	bool found = false;
	for (uint32_t i = 0; i < MAX_PTR_CHECKS; i++) {
		if (_pointers[i].loc == p) {
			found = true;
			_pointers[i].loc = NULL;
			break;
		}
	}
	if (!found) {
		free_without_malloc++;
	}

	update_allocations();
}
