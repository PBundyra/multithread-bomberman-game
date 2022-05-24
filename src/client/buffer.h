#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <iostream>
#include "common.h"
#include "err.h"

#define MAX_DATAGRAM_SIZE 65507

class Buffer {
private:
    char buffer[MAX_DATAGRAM_SIZE]{};
    size_t no_bytes;
    size_t no_bytes_read;

public:
    Buffer() : no_bytes(0), no_bytes_read(0) {};

    char *get_buffer() {
        return buffer;
    }

    void write_into_buffer(uint8_t msg);

    void write_into_buffer(uint16_t msg);

    void write_into_buffer(uint32_t msg);

    void write_into_buffer(uint64_t msg);

    void write_str_into_buffer(std::string &msg);

    [[nodiscard]] size_t get_no_bytes() const;

    std::string cpy_buffer();

    uint8_t read_1_byte();

    uint16_t read_2_bytes();

    uint32_t read_4_bytes();

    uint64_t read_8_bytes();

    std::string read_str(const size_t len);
};


#endif //BUFFER_H
