#include <iostream>
#include <string.h>
#include <sys/resource.h>

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


