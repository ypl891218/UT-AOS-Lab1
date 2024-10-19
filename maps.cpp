#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

using namespace std;

void printTimeval(const timeval &tv) {
    std::cout << "Seconds: " << tv.tv_sec 
              << ", Microseconds: " << tv.tv_usec 
              << std::endl;
}

int printProcMap() {
    std::ifstream maps_file("/proc/self/maps");
    
    if (!maps_file) {
        std::cerr << "Error: Could not open /proc/self/maps" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(maps_file, line)) {
        std::cout << line << std::endl;
    }

    maps_file.close();
    return 0;
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

    std::cout << "Max resident set size: " << usage.ru_maxrss << endl;
    std::cout << "Page reclaims (soft page faults): " << usage.ru_minflt << endl;
    std::cout << "Page faults (hard page faults): " << usage.ru_majflt << endl;
    std::cout << "Block input operations: " << usage.ru_inblock << endl;
    std::cout << "Block output operations: " << usage.ru_oublock << endl;
    std::cout << "Voluntary context switches: " << usage.ru_nvcsw << endl;
    std::cout << "Involuntary context switches: " << usage.ru_nivcsw << endl;
    return 0;
}

int perf_event_open(struct perf_event_attr *pe, pid_t pid, int cpu,
		    int group_fd, unsigned long flags) {
    int ret = syscall(SYS_perf_event_open, pe, pid, cpu, group_fd, flags);
    if (ret < 0) {
        std::cerr << "Error calling syscall: " << strerror(errno) << std::endl;
    }
    return ret;
}

enum class PerfEvents {
    L1_DATA_HIT,
    L1_DATA_MISS,
    DATA_TLB_MISS,
};

const std::unordered_map<std::string, PerfEvents> stringToPerfEvent = {
    {"L1_DATA_HIT", PerfEvents::L1_DATA_HIT},
    {"L1_DATA_MISS", PerfEvents::L1_DATA_MISS},
    {"DATA_TLB_MISS", PerfEvents::DATA_TLB_MISS}
};

int getFdPerfEventOpen(PerfEvents perf_event) {
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(perf_event_attr));
    pe.size = sizeof(struct perf_event_attr);
    switch (perf_event) {
	case PerfEvents::L1_DATA_HIT:
            pe.type = PERF_TYPE_HW_CACHE;
	    pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | \
			(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
            break;
	case PerfEvents::L1_DATA_MISS:
	    pe.type = PERF_TYPE_HW_CACHE;
	    pe.config = (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | \
			(PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
	    break;
	case PerfEvents::DATA_TLB_MISS:
	    pe.type = PERF_TYPE_HW_CACHE;
	    pe.config = (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | \
			(PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
            break;
        default:
	    std::cout << "Unsupported perf events to monitor" << std::endl;
	    return -1;
    }
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    return perf_event_open(&pe, 0, -1, -1, 0);
}

int main(int argc, char **argv) {
    if (argc <= 1 || stringToPerfEvent.find(argv[1]) == stringToPerfEvent.end()) {
	std::cerr << "usage: " << argv[0] << " " << "[L1_DATA_HIT / L1_DATA_MISS / DATA_TLB_MISS]" << std::endl;
        return -1;
    }

    int ret = -1;
    std::string argPerfEvent(argv[1]);
    int fd_perf_event = getFdPerfEventOpen(stringToPerfEvent.at(argPerfEvent));
    if (fd_perf_event == -1) {
        return -1;
    }
    ioctl(fd_perf_event, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_perf_event, PERF_EVENT_IOC_ENABLE, 0);
    if (printProcMap() != 0) {
        goto End;
    }
    if (printRUsage() != 0) {
        goto End;
    }
    ioctl(fd_perf_event, PERF_EVENT_IOC_DISABLE, 0);

    long long event_count;
    if (read(fd_perf_event, &event_count, sizeof(event_count)) == -1) {
	std::cerr << "Error reading perf event: " << strerror(errno) << std::endl;
        goto End;
    }

    std::cout << argv[1] << ": " << event_count << std::endl;
    ret = 0;
End:
    close(fd_perf_event);
    return ret;
}
