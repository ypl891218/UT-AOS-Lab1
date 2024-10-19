#include <iostream>
#include <unistd.h>     // For syscall() and close()
#include <sys/ioctl.h>  // For ioctl()
#include <sys/syscall.h> // For SYS_perf_event_open
#include <linux/perf_event.h> // For perf_event_attr
#include <cstring>      // For memset()
#include <cerrno>       // For errno handling
#include <stdexcept>    // For exceptions

// Wrapper for perf_event_open syscall
int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, 
                    int group_fd, unsigned long flags) {
    return syscall(SYS_perf_event_open, attr, pid, cpu, group_fd, flags);
}

int main() {
    // Initialize the perf_event_attr structure
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    
    pe.type = PERF_TYPE_HW_CACHE;  // Hardware cache events
    pe.size = sizeof(struct perf_event_attr);
    pe.config = (PERF_COUNT_HW_CACHE_DTLB |    // Data TLB
                 (PERF_COUNT_HW_CACHE_OP_READ << 8) |  // Read operation
                 (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));  // Misses
    pe.disabled = 1;  // Start in disabled state
    pe.exclude_kernel = 1;  // Exclude kernel events
    pe.exclude_hv = 1;  // Exclude hypervisor events

    // Open the performance event counter
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
        std::cerr << "Error opening perf event: " << strerror(errno) << std::endl;
        return 1;
    }

    // Start counting
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

    // Some sample code to trigger TLB activity
    const int size = 1024 * 1024;  // 1 million integers (~4 MB)
    int *array = new int[size];
    for (int i = 0; i < size; i += 64) {  // Stride access to cause TLB pressure
        array[i] = i;
    }
    delete[] array;

    // Stop counting
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

    // Read the counter value
    long long tlb_misses;
    if (read(fd, &tlb_misses, sizeof(tlb_misses)) == -1) {
        std::cerr << "Error reading perf event: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }

    std::cout << "Data TLB Misses: " << tlb_misses << std::endl;

    // Clean up
    close(fd);
    return 0;
}

