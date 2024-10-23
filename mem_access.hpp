#ifndef LAB1_MEM_ACCESS
#define LAB1_MEM_ACCESS

#define CACHE_LINE_SIZE 64

void do_mem_access(char* p, int size, bool opt_random_access);
int compete_for_memory();

#endif
