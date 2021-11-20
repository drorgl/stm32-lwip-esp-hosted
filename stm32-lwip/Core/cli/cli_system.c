#include "cli_system.h"
#include "cli.h"

#include <stdio.h>
#include <stdbool.h>

#include <malloc.h>
#include "heapinfo.h"


#include "FreeRTOS.h"
#include "portable.h"

static void display_mallinfo2(void) {
	struct mallinfo mi;

	mi = mallinfo();

	printf("Total non-mmapped bytes (arena):       %lu\n", mi.arena);
	printf("# of free chunks (ordblks):            %lu\n", mi.ordblks);
	printf("# of free fastbin blocks (smblks):     %lu\n", mi.smblks);
	printf("# of mapped regions (hblks):           %lu\n", mi.hblks);
	printf("Bytes in mapped regions (hblkhd):      %lu\n", mi.hblkhd);
	printf("Max. total allocated space (usmblks):  %lu\n", mi.usmblks);
	printf("Free bytes held in fastbins (fsmblks): %lu\n", mi.fsmblks);
	printf("Total allocated space (uordblks):      %lu\n", mi.uordblks);
	printf("Total free space (fordblks):           %lu\n", mi.fordblks);
	printf("Topmost releasable block (keepcost):   %lu\n", mi.keepcost);
}

static void display_freertos_heap_stats(void) {
	HeapStats_t stat;
	vPortGetHeapStats(&stat);
	/* The total heap size currently available - this is the sum of all the free blocks, not the largest block that can be allocated. */
	printf("Available Heap Space %d bytes\r\n", stat.xAvailableHeapSpaceInBytes);
	/* The maximum size, in bytes, of all the free blocks within the heap at the time vPortGetHeapStats() is called. */
	printf("Size of largest free block %d bytes\r\n",
			stat.xSizeOfLargestFreeBlockInBytes);
	/* The minimum size, in bytes, of all the free blocks within the heap at the time vPortGetHeapStats() is called. */
	printf("Size of smallest free block %d bytes\r\n",
			stat.xSizeOfSmallestFreeBlockInBytes);
	/* The number of free memory blocks within the heap at the time vPortGetHeapStats() is called. */
	printf("Number of free blocks %d\r\n", stat.xNumberOfFreeBlocks);
	/* The minimum amount of total free memory (sum of all free blocks) there has been in the heap since the system booted. */
	printf("Minimum Ever Free Bytes Remaining %d\r\n",
			stat.xMinimumEverFreeBytesRemaining);
	/* The number of calls to pvPortMalloc() that have returned a valid memory block. */
	printf("Number of Successful Allocations %d\r\n",
			stat.xNumberOfSuccessfulAllocations);
	printf("Number of Successful Frees %d\r\n", stat.xNumberOfSuccessfulFrees);
}

static void freemem_callback(int argc, const char *const*argv,
		void (*print)(const char *message)) {

	printf("malloc stats:\r\n");

	malloc_stats();

	display_mallinfo2();

	printf("Total RAM %d, Available %d\r\n", get_total_heap_memory(),
			get_available_heap_memory());

	display_freertos_heap_stats();
}

void cli_system_register(void) {
	cli_register("freemem", "freemem - show memory info", freemem_callback);
}
