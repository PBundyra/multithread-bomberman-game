#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <iostream>
#include "common.h"
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

    size_t get_no_written_bytes() const;

    void reset_buffer();

    void print_buffer();


//    std::string cpy_buffer();

    void write_into_buffer(const uint8_t msg);

    void write_into_buffer(const uint16_t msg);

    void write_into_buffer(const uint32_t msg);

    void write_into_buffer(const uint64_t msg);

    void write_into_buffer(const char *msg, const size_t len);

    void overwrite_buffer(const uint8_t msg, const size_t pos);

    void overwrite_buffer(const uint16_t msg, const size_t pos);

    void overwrite_buffer(const uint32_t msg, const size_t pos);

    void overwrite_buffer(const uint64_t msg, const size_t pos);

    void overwrite_buffer(const char *msg, const size_t len, const size_t pos);

    uint8_t read_1_byte();

    uint16_t read_2_bytes();

    uint32_t read_4_bytes();

    uint64_t read_8_bytes();

    std::string read_n_bytes(const size_t len);
};


#endif //BUFFER_H
