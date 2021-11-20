#pragma once
#include <stdbool.h>
#include <stddef.h>

#define MAX_PTR_CHECKS 100


extern int multiple_allocations;
extern int free_without_malloc;

void add_alloc(void *p, size_t size) ;

void remove_alloc(void *p);
