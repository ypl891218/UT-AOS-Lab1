#include <iostream>
#include <cstdlib>
#include <memory>
#include <string.h>
#include <unistd.h>
#include <sched.h>

#include "utils.hpp"
#include "mem_access.hpp"
#include "perf_event_open.hpp"

int main(int argc, char** argv) {
    int ret = -1;
    int fd_l1_access = -1;
    int fd_l1_miss = -1;
    int fd_tlb_miss = -1;
    int cpu_id = -1;
    bool random = true;

    if (argc >= 2) {
    	cpu_id = atoi(argv[1]);
    }
    if (argc >= 3) {
    	random = atoi(argv[2]);
    }
    std::cout << random << std::endl;
    std::unique_ptr<char[]> buffer;
    
    if (fixToCPU0() != 0) {
    	return ret;
    }

    if ((fd_l1_access = getFdPerfEventOpen(PerfEvents::L1_DATA_READ_ACCESS, cpu_id)) == -1 ||
        (fd_l1_miss = getFdPerfEventOpen(PerfEvents::L1_DATA_READ_MISS, cpu_id)) == -1 ||
        (fd_tlb_miss = getFdPerfEventOpen(PerfEvents::DATA_TLB_READ_MISS, cpu_id)) == -1) {
        std::cerr << "Failed to get fd for three perf events" << std::endl;
        return ret;
    }

    if (beginRecordPerfEvent(fd_l1_access) != 0 ||
        beginRecordPerfEvent(fd_l1_miss) != 0 ||
		beginRecordPerfEvent(fd_tlb_miss) != 0) {
        goto End;
    }

    buffer = std::make_unique<char[]>(1<<30);
    do_mem_access(buffer.get(), 1<<30, random);

    if (endRecordPerfEvent(fd_l1_access) != 0 ||
		endRecordPerfEvent(fd_l1_miss) != 0 ||
		endRecordPerfEvent(fd_tlb_miss) != 0) {
        goto End;
    }

    long long l1_access_cnt, l1_miss_cnt, tlb_miss_cnt;
    if (getResultPerfEvent(fd_l1_access, l1_access_cnt) != 0 ||
        getResultPerfEvent(fd_l1_miss, l1_miss_cnt) != 0 ||
        getResultPerfEvent(fd_tlb_miss, tlb_miss_cnt) != 0) {
        goto End;
    } 

    std::cout << l1_access_cnt << "," << l1_miss_cnt << "," << tlb_miss_cnt << ",";
    // std::cout << "L1 Accesses: " << l1_access_cnt << std::endl;
    // std::cout << "L1 Misses: " << l1_miss_cnt << std::endl;
    //std::cout << "TLB Misses: " << tlb_miss_cnt << std::endl;

    printRUsageComma();
    //printRUsage();
    ret = 0;

End:
    if (fd_l1_access != -1) {
        closeFdPerfEvent(fd_l1_access);
    }
    if (fd_l1_miss != -1) {
        closeFdPerfEvent(fd_l1_miss);
    }
    if (fd_tlb_miss != -1) {
        closeFdPerfEvent(fd_tlb_miss);
    }
    return ret;
}
