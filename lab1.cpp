#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <unistd.h>
#include "utils.hpp"
#include "perf_event_open.hpp"

using namespace std;

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

int main(int argc, char **argv) {
    PerfEvents pe_type;
    if (argc <= 1 || (pe_type = getPerfEventType(argv[1])) == PerfEvents::UNSUPPORTED) {
	printPerfEventUsage();
	return -1;
    }

    int ret = -1;
    int fd_perf_event = getFdPerfEventOpen(pe_type);
    if (fd_perf_event == -1) {
        return -1;
    }

    if (beginRecordPerfEvent(fd_perf_event) != 0) {
        goto End;
    }

    if (printProcMap() != 0) {
        goto End;
    }
    if (printRUsage() != 0) {
        goto End;
    }

    if (endRecordPerfEvent(fd_perf_event) != 0) {
        goto End;
    }

    long long event_count;
    if (getResultPerfEvent(fd_perf_event, event_count) != 0) {
        goto End;
    } 

    std::cout << argv[1] << ": " << event_count << std::endl;
    ret = 0;
End:
    closeFdPerfEvent(fd_perf_event);
    return ret;
}
