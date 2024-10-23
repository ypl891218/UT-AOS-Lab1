#include <cstddef>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include "mem_access.hpp"

long x = 1, y = 4, z = 7, w = 13;

long simplerand(void) {
	long t = x;
	t ^= t << 11;
	t ^= t >> 8;
	x = y;
	y = z;
	z = w;
	w ^= w >> 19;
	w ^= t;
	return w;
}

void do_mem_access(char* p, int size, bool opt_random_access) {
   int i, outer, locality;
   int ws_base = 0;
   int max_base = ((size / CACHE_LINE_SIZE) - 512);
	for(outer = 0; outer < (1<<20); ++outer) {
      long r = simplerand() % max_base;
      // Pick a starting offset
      if( opt_random_access ) {
         ws_base = r;
      } else {
         ws_base += 512;
         if( ws_base >= max_base ) {
            ws_base = 0;
         }
      }
      for(locality = 0; locality < 16; locality++) {
         volatile char *a;
         char c;
	 for(i = 0; i < 512; i++) {
            // Working set of 512 cache lines, 32KB
            a = p + (ws_base + i) * CACHE_LINE_SIZE;
            if((i%8) == 0) {
               *a = 1;
            } else {
               c = *a;
            }
         }
      }
   }
}
static int get_mem_size() {
    struct sysinfo info;
    sysinfo(&info);

    return info.totalram;
}

int compete_for_memory() {
   long mem_size = get_mem_size();
   int page_sz = sysconf(_SC_PAGE_SIZE);
   printf("Total memsize is %3.2f GBs\n", (double)mem_size/(1024*1024*1024));
   fflush(stdout);
   char* p = (char*)mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
                  MAP_NORESERVE|MAP_PRIVATE|MAP_ANONYMOUS, -1, (off_t) 0);
   if (p == MAP_FAILED)
      perror("Failed anon MMAP competition");

   int i = 0;
   while(1) {
      volatile char *a;
      long r = simplerand() % (mem_size/page_sz);
      char c;
      if( i >= mem_size/page_sz ) {
         i = 0;
      }
      // One read and write per page
      //a = p + i * page_sz; // sequential access
      a = p + r * page_sz;
      c += *a;
      if((i%8) == 0) {
         *a = 1;
      }
      i++;
   }
   return 0;
}

