#ifndef LAB1_PERF_EVENT_OPEN
#define LAB1_PERF_EVENT_OPEN

#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <string>

enum class PerfEvents {
    L1_DATA_READ_ACCESS,
    L1_DATA_WRITE_ACCESS,
    L1_DATA_PREFETCH_ACCESS,
    L1_DATA_READ_MISS,
    L1_DATA_WRITE_MISS,
    L1_DATA_PREFETCH_MISS,
    DATA_TLB_READ_MISS,
    DATA_TLB_WRITE_MISS,
    DATA_TLB_PREFETCH_MISS,
    UNSUPPORTED,
};
void printPerfEventUsage();
PerfEvents getPerfEventType(const std::string &str);

int getFdPerfEventOpen(PerfEvents perf_event);

int beginRecordPerfEvent(int fd);
int endRecordPerfEvent(int fd);
int getResultPerfEvent(int fd, long long &event_result);

int closeFdPerfEvent(int fd);

#endif
