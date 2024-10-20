#include <iostream>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "mem_access.hpp"
#include "perf_event_open.hpp"
#include "utils.hpp"

static void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options]\n"
              << "Options:\n"
              << "  -s                Use sequential access\n"
              << "  -r                Use random access\n"
              << "                    (Mutually exclusive with -s)\n"
              << "  -a                Use anonymous mapping\n"
              << "  -f <filename>     Use file-backed mapping\n"
              << "                    (Mutually exclusive with -a)\n"
              << "  -P                Use MAP_POPULATE\n"
              << "                    (Mutually exclusive with -a)\n"
              << "  -p                Use MAP_PRIVATE\n"
              << "                    (Mutually exclusive with -m)\n"
              << "  -m                Use MAP_SHARED\n"
              << "  -h                Show this help message\n";
}

static int parse_argv(int argc, char **argv, std::string &file_name, bool &sequential, bool &random, bool &anonymous,
                    bool &file_backed, bool &private_mapping, bool &shared_mapping, bool &populate) {
    file_name.clear();
    sequential = false, random = false;
    anonymous = false, file_backed = false;
    private_mapping = false, shared_mapping = false, populate = false;

    int opt;
    while ((opt = getopt(argc, argv, "f:srampP")) != -1) {
        switch (opt) {
            case 'h':
                return 1;

            // Sequential vs. Random (mutually exclusive)
            case 's':
                if (random) {
                    std::cerr << "Error: Options -s (sequential) and -r (random) are mutually exclusive.\n";
                    return 1;
                }
                sequential = true;
                break;

            case 'r':
                if (sequential) {
                    std::cerr << "Error: Options -s (sequential) and -r (random) are mutually exclusive.\n";
                    return 1;
                }
                random = true;
                break;

            // Anonymous vs. File-Backed (mutually exclusive)
            case 'a':
                if (file_backed || populate) {
                    std::cerr << "Error: Options -a (anonymous) and -f (file-backed) are mutually exclusive.\n";
                    return 1;
                }
                anonymous = true;
                break;

            case 'f':
                if (anonymous) {
                    std::cerr << "Error: Options -a (anonymous) and -f (file-backed) are mutually exclusive.\n";
                    return 1;
                }
                file_backed = true;
                file_name = optarg;
                break;

            // MAP_PRIVATE vs. MAP_SHARED (mutually exclusive)
            case 'p':
                if (shared_mapping) {
                    std::cerr << "Error: Options -p (private) and -m (shared) are mutually exclusive.\n";
                    return 1;
                }
                private_mapping = true;
                break;

            case 'm':
                if (private_mapping) {
                    std::cerr << "Error: Options -p (private) and -m (shared) are mutually exclusive.\n";
                    return 1;
                }
                shared_mapping = true;
                break;

            // MAP_POPULATE (exclusive with anonymous)
            case 'P':
                if (anonymous) {
                    std::cerr << "Error: Options -P (Populate) and -a (anonymous) are mutually exclusive.\n";
                }
                populate = true;
                break;

            // Invalid option handling
            default:
                std::cerr << "Invalid option. Use -h for help.\n";
                return 1;
        }
    }

    if (!sequential && !random) {
        std::cerr << "Error: Either -s (sequential) or -r (random) is required\n";
        return 1;
    } 
    if (!anonymous && !file_backed) {
        std::cerr << "Error: Either -a (anonymous) or -f (file_backed) is required\n";
        return 1;
    }
    if (file_backed && !shared_mapping && !private_mapping) {
        std::cerr << "Error: When using -f (file_backed), either -p (private_mapping) or -m (shared_mapping) is required\n";
        return 1;
    }

    return 0;
}


int main(int argc, char** argv) {
    bool sequential = false, random = false;
    bool anonymous = false, file_backed = false;
    bool private_mapping = false, shared_mapping = false, populate = false;
    std::string file_name;

    int flags = 0;
    int fd_file = -1;
    size_t mmap_size = 1L << 30;  // 1 GB
    void* addr = nullptr;
    int fd_l1_access = -1, fd_l1_miss = -1, fd_tlb_miss = -1;
    long long l1_access_cnt, l1_miss_cnt, tlb_miss_cnt;
    int ret = -1;

    if (0 != parse_argv(argc, argv, file_name, sequential, random, anonymous, file_backed, private_mapping, shared_mapping, populate)) {
        print_usage(argv[0]);
        return ret;
    }

    if (anonymous) {
        flags |= MAP_ANONYMOUS | MAP_PRIVATE;
        std::cout << "anonymous\n";
    } else {
        if (private_mapping) {
            flags |= MAP_PRIVATE;
        } else if (shared_mapping) {
            flags |= MAP_SHARED;
        }
 
        if (populate) {
            flags |= MAP_POPULATE;
        }
    }

    if (file_backed) {
        fd_file = open(file_name.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd_file == -1) {
            std::cerr << "Error opening file: " << strerror(errno) << "\n";
            goto End;
        }
    } 

    // Call mmap()
    addr = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE, flags, fd_file, 0);
    if (addr == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << "\n";
        goto End;
    }

    std::cout << "mmap succeeded. Address: " << addr << "\n";

    if ((fd_l1_access = getFdPerfEventOpen(PerfEvents::L1_DATA_READ_ACCESS)) == -1 ||
        (fd_l1_miss = getFdPerfEventOpen(PerfEvents::L1_DATA_READ_MISS)) == -1 ||
        (fd_tlb_miss = getFdPerfEventOpen(PerfEvents::DATA_TLB_READ_MISS)) == -1) {
        std::cerr << "Failed to get fd for three perf events" << std::endl;
        goto End;
    }

    if (beginRecordPerfEvent(fd_l1_access) != 0 ||
        beginRecordPerfEvent(fd_l1_miss) != 0 ||
		beginRecordPerfEvent(fd_tlb_miss) != 0) {
        goto End;
    }

    do_mem_access((char*)addr, mmap_size, random);

    if (endRecordPerfEvent(fd_l1_access) != 0 ||
		endRecordPerfEvent(fd_l1_miss) != 0 ||
		endRecordPerfEvent(fd_tlb_miss) != 0) {
        goto End;
    }

    if (getResultPerfEvent(fd_l1_access, l1_access_cnt) != 0 ||
        getResultPerfEvent(fd_l1_miss, l1_miss_cnt) != 0 ||
        getResultPerfEvent(fd_tlb_miss, tlb_miss_cnt) != 0) {
        goto End;
    } 

    std::cout << "L1 Accesses: " << l1_access_cnt << std::endl;
    std::cout << "L1 Misses: " << l1_miss_cnt << std::endl;
    std::cout << "TLB Misses: " << tlb_miss_cnt << std::endl;

    printRUsage();

    ret = 0;
End:
    if (fd_file != -1) {
        close (fd_file);
    }
    if (addr != MAP_FAILED && addr != nullptr) {
        munmap(addr, mmap_size);
    }
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
