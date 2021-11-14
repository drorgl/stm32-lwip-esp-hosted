#include "cli_system.h"
#include "cli.h"

#include <stdio.h>
#include <stdbool.h>

#include <malloc.h>
#include "heapinfo.h"

static void
       display_mallinfo2(void)
       {
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

static void freemem_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	printf("malloc stats:\r\n");

	malloc_stats();

	display_mallinfo2();

	printf("Total RAM %d, Available %d\r\n", get_total_heap_memory(),get_available_heap_memory() );
}

void cli_system_register(void){
	cli_register("freemem", "freemem - show memory info", freemem_callback);
}
