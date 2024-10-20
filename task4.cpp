#include <iostream>
#include <unistd.h>

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

static int parseArgv(int argc, char **argv, string &file_name, bool &sequential, bool &random, bool &anonymous,
                    bool &file_backed, bool &private_mapping, bool &shared_mapping, bool &populate) {
    file_name.clear();
    sequential = false, random = false;
    anonymous = false, file_backed = false;
    private_mapping = false, shared_mapping = false, populate = false;

    int opt;
    while ((opt = getopt(argc, argv, "f:srampP")) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return 0;

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
    return 0;
}


int main(int argc, char** argv) {
    bool sequential = false, random = false;
    bool anonymous = false, file_backed = false;
    bool private_mapping = false, shared_mapping = false, populate = false;

    if (0 != parseArgv(argc, argv, sequential, random, anonymous, file_backed, private_mapping, shared_mapping, populate)) {
        return 1;
    }

    int flags = 0;
    if (anonymous) {
        flags |= MAP_ANONYMOUS;
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

    int fd = -1;
    if (file_backed) {
        fd = open(filename, O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            std::cerr << "Error opening file: " << strerror(errno) << "\n";
            return 1;
        }
    } 

    size_t mmap_size = 1L << 30;  // 1 GB

    // Call mmap()
    void* addr = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE, flags, fd, 0);
    if (addr == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << "\n";
        if (fd != -1) close(fd);
        return 1;
    }

    std::cout << "mmap succeeded. Address: " << addr << "\n";

    
}
