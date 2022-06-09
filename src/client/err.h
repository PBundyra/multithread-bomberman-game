#ifndef MIMUW_SIK_ERR_H
#define MIMUW_SIK_ERR_H

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cerrno>

class Err {
public:
    // Evaluate `x`: if false, print an error message and exit with an error.
    static void ensure(bool x) {
        if (!x) {
            std::cerr << "Error: the statement was false in " << __func__ << "at " << __FILE__ << ":" << __LINE__
                      << "\n";
            exit(EXIT_FAILURE);
        }
    }

    // Check if errno is non-zero, and if so, print an error message and exit with an error.
    static void print_errno() {
        if (errno != 0) {
            std::cerr << "Error: errno " << errno << " in " << __func__ << " at " << __FILE__ << ":" << __LINE__ << "\n"
                      << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
    }

    static void check_errno(int x) {
        if (x != 0) {
            print_errno();
        }
    }

    // Print an error message and exit with an error.
    template<typename Args>
    static void fatal(Args &&args...) {
        std::cerr << "Error: ";
        std::cerr << std::forward<Args>(args) << "\n";
        exit(EXIT_FAILURE);
    }
};

#endif // MIMUW_SIK_ERR_H
