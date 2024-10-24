#include <cstring>
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>

#include "perf_event_open.hpp"

static int perf_event_open(struct perf_event_attr *pe, pid_t pid, int cpu,
		    int group_fd, unsigned long flags) {
    int ret = syscall(SYS_perf_event_open, pe, pid, cpu, group_fd, flags);
    if (ret < 0) {
        std::cerr << "Error calling syscall: " << strerror(errno) << std::endl;
    }
    return ret;
}

const std::unordered_map<std::string, PerfEvents> stringToPerfEvent = {
    {"L1_DATA_READ_ACCESS", PerfEvents::L1_DATA_READ_ACCESS},
    {"L1_DATA_WRITE_ACCESS", PerfEvents::L1_DATA_WRITE_ACCESS},
    {"L1_DATA_PREFETCH_ACCESS", PerfEvents::L1_DATA_PREFETCH_ACCESS},
    {"L1_DATA_READ_MISS", PerfEvents::L1_DATA_READ_MISS},
    {"L1_DATA_WRITE_MISS", PerfEvents::L1_DATA_WRITE_MISS},
    {"L1_DATA_PREFETCH_MISS", PerfEvents::L1_DATA_PREFETCH_MISS},
    {"DATA_TLB_READ_MISS", PerfEvents::DATA_TLB_READ_MISS},
    {"DATA_TLB_WRITE_MISS", PerfEvents::DATA_TLB_WRITE_MISS},
    {"DATA_TLB_PREFETCH_MISS", PerfEvents::DATA_TLB_PREFETCH_MISS},
};

void printPerfEventUsage() {
    std::cerr << "Available perf event types: [ ";
    for (auto &[str, _]: stringToPerfEvent) {
        std::cerr << str << " / ";
    }
    std::cerr << "]" << std::endl;
}

PerfEvents getPerfEventType(const std::string &str) {
    if (stringToPerfEvent.find(str) == stringToPerfEvent.end()) {
        return PerfEvents::UNSUPPORTED;
    }
    return stringToPerfEvent.at(str);
}

int getFdPerfEventOpen(PerfEvents perf_event, int cpu_id) {
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(perf_event_attr));
    pe.size = sizeof(struct perf_event_attr);

    switch (perf_event) {
        case PerfEvents::L1_DATA_READ_ACCESS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
            break;
        case PerfEvents::L1_DATA_WRITE_ACCESS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
            break;
        case PerfEvents::L1_DATA_PREFETCH_ACCESS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
            break;
        case PerfEvents::L1_DATA_READ_MISS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        case PerfEvents::L1_DATA_WRITE_MISS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        case PerfEvents::L1_DATA_PREFETCH_MISS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        case PerfEvents::DATA_TLB_READ_MISS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        case PerfEvents::DATA_TLB_WRITE_MISS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        case PerfEvents::DATA_TLB_PREFETCH_MISS:
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | 
                        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        default:
            std::cout << "Unsupported perf events to monitor" << std::endl;
            return -1;
    }


    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    return perf_event_open(&pe, 0, cpu_id, -1, 0);
}


int beginRecordPerfEvent(int fd_perf_event) {
    if (ioctl(fd_perf_event, PERF_EVENT_IOC_RESET, 0) != 0) {
        std::cerr << "Error calling ioctl PERF_EVENT_IOC_RESET: " << strerror(errno) << std::endl;
	return -1;
    }
    if (ioctl(fd_perf_event, PERF_EVENT_IOC_ENABLE, 0) != 0) {
        std::cerr << "Error calling ioctl PERF_EVENT_IOC_ENABLE: " << strerror(errno) << std::endl;
	return -1;
    }
    return 0;
}

int endRecordPerfEvent(int fd_perf_event) {
    if (ioctl(fd_perf_event, PERF_EVENT_IOC_DISABLE, 0) != 0) {
        std::cerr << "Error calling ioctl PERF_EVENT_IOC_DISABLE: " << strerror(errno) << std::endl;
	return -1;
    }
    return 0;
}

int getResultPerfEvent(int fd_perf_event, long long& event_result) {
    if (read(fd_perf_event, &event_result, sizeof(event_result)) == -1) {
	std::cerr << "Error reading perf event: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int closeFdPerfEvent(int fd) {
    return close(fd);
}


