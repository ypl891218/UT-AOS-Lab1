#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <cstring>
#include <cstdlib>

// Wrapper for perf_event_open syscall
int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                    int cpu, int group_fd, unsigned long flags) {
    return syscall(SYS_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int main() {
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));

    // Configure the event to count CPU cycles
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    pe.disabled = 1;
    pe.exclude_kernel = 1;  // Don't count kernel events
    pe.exclude_hv = 1;      // Don't count hypervisor events

    // Open the perf event counter
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
        std::cerr << "Error opening perf event: " << strerror(errno) << std::endl;
        return 1;
    }

    // Start counting
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

    // Some code to measure
    for (volatile int i = 0; i < 100000000; ++i);  // Busy loop

    // Stop counting
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

    // Read the counter value
    long long count;
    read(fd, &count, sizeof(long long));

    std::cout << "CPU cycles: " << count << std::endl;

    close(fd);
    return 0;
}

