#include <iostream>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

void printTimeval(const timeval &tv) {
    std::cout << "Seconds: " << tv.tv_sec 
              << ", Microseconds: " << tv.tv_usec 
              << std::endl;
}

int printRUsage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        std::cerr << "Error calling getrusage: " << strerror(errno) << std::endl;
       return 1;	
    }
    
    std::cout << "User time: ";
    printTimeval(usage.ru_utime);
    
    std::cout << "System time: ";
    printTimeval(usage.ru_stime);

    std::cout << "Max resident set size: " << usage.ru_maxrss << std::endl;
    std::cout << "Page reclaims (soft page faults): " << usage.ru_minflt << std::endl;
    std::cout << "Page faults (hard page faults): " << usage.ru_majflt << std::endl;
    std::cout << "Block input operations: " << usage.ru_inblock << std::endl;
    std::cout << "Block output operations: " << usage.ru_oublock << std::endl;
    std::cout << "Voluntary context switches: " << usage.ru_nvcsw << std::endl;
    std::cout << "Involuntary context switches: " << usage.ru_nivcsw << std::endl;
    return 0;
}

int fixToCPU0() {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) != 0) {
	std::cerr << "sched_getaffinity() failed: " << strerror(errno) << std::endl;
	return 1;
    }
    std::cout << sched_getcpu() << std::endl;
    return 0;
}

void print_affinity() {
    cpu_set_t mask;
    long nproc, i;

    sched_getaffinity(0, sizeof(cpu_set_t), &mask);
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    printf("sched_getaffinity = ");
    for (i = 0; i < nproc; i++) {
        printf("%d ", CPU_ISSET(i, &mask));
    }
    printf("\n");
}
