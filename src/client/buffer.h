#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <iostream>
#include <cstring>
#include "err.h"

#define MAX_DATAGRAM_SIZE 65507
#define BUFFER_SIZE MAX_DATAGRAM_SIZE

class Buffer {
private:
    char buffer[MAX_DATAGRAM_SIZE]{};
    size_t no_bytes_written;
    size_t no_bytes_read;

public:
    Buffer() : no_bytes_written(0), no_bytes_read(0) {};

    char *get_buffer();

    [[nodiscard]] size_t get_no_written_bytes() const;

    void reset_buffer();

    static void print_buffer(const char *msg, size_t len);

    void write_into_buffer(uint8_t msg);

    void write_into_buffer(uint16_t msg);

    void write_into_buffer(uint32_t msg);

    void write_into_buffer(uint64_t msg);

    void write_into_buffer(const char *msg, size_t len);

    void overwrite_buffer(uint8_t msg, size_t pos);

    void overwrite_buffer(uint16_t msg, size_t pos);

    void overwrite_buffer(uint32_t msg, size_t pos);

    void overwrite_buffer(uint64_t msg, size_t pos);

    void overwrite_buffer(const char *msg, size_t len, size_t pos);

    uint8_t read_1_byte();

    uint16_t read_2_bytes();

    uint32_t read_4_bytes();

    uint64_t read_8_bytes();

    std::string read_n_bytes(size_t len);
};

#endif //BUFFER_H
