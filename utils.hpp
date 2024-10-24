#ifndef LAB1_UTILS_HPP
#define LAB1_UTILS_HPP

#include <sys/time.h>

void printTimeval(const timeval &tv);
int printRUsageComma();
int printRUsage();
int fixToCPU0();
void print_affinity();
#endif
