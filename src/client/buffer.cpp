#include "buffer.h"

#include <bitset>
#include <iostream>
#include <cstring>
#include "err.h"

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

char *Buffer::get_buffer() {
    return buffer;
}

size_t Buffer::get_no_written_bytes() const {
    return no_bytes_written;
}

void Buffer::reset_buffer() {
    no_bytes_written = 0;
    no_bytes_read = 0;
    memset(buffer, 0, BUFFER_SIZE);
}

void Buffer::print_buffer(const char *msg, const size_t len) {
    if (debug) {
        for (size_t i = 0; i < len; ++i) {
            std::cerr << (int) msg[i] << " " << msg[i] << " ";
            std::bitset<8> bs4(msg[i]);
            std::cerr << bs4 << " ";

            if (i % 6 == 0 && i != 0)
                std::cerr << std::endl;
        }
        std::cerr << std::endl;
    }
}

void Buffer::write_into_buffer(const uint8_t msg) {
    auto val = msg;
    memcpy(buffer + no_bytes_written, &val, sizeof(uint8_t));
    no_bytes_written += sizeof(uint8_t);
}

void Buffer::write_into_buffer(const uint16_t msg) {
    auto val = msg;
    memcpy(buffer + no_bytes_written, &val, sizeof(uint16_t));
    no_bytes_written += sizeof(uint16_t);
}

void Buffer::write_into_buffer(const uint32_t msg) {
    auto val = msg;
    memcpy(buffer + no_bytes_written, &val, sizeof(uint32_t));
    no_bytes_written += sizeof(uint32_t);
}

void Buffer::write_into_buffer(const uint64_t msg) {
    auto val = msg;
    memcpy(buffer + no_bytes_written, &val, sizeof(uint64_t));
    no_bytes_written += sizeof(uint64_t);
}

void Buffer::write_into_buffer(const char *msg, const size_t len) {
    for (size_t i = 0; i < len; i++) {
        write_into_buffer((uint8_t) msg[i]);
    }
}

uint8_t Buffer::read_1_byte() {
    uint8_t val;
    memcpy(&val, buffer + no_bytes_read, sizeof(uint8_t));
    no_bytes_read += sizeof(uint8_t);
    return val;
}

uint16_t Buffer::read_2_bytes() {
    uint16_t val;
    memcpy(&val, buffer + no_bytes_read, sizeof(uint16_t));
    no_bytes_read += sizeof(uint16_t);
    return be16toh(val);
}

uint32_t Buffer::read_4_bytes() {
    uint32_t val;
    memcpy(&val, buffer + no_bytes_read, sizeof(uint32_t));
    no_bytes_read += sizeof(uint32_t);
    return be32toh(val);
}

uint64_t Buffer::read_8_bytes() {
    uint64_t val;
    memcpy(&val, buffer + no_bytes_read, sizeof(uint64_t));
    no_bytes_read += sizeof(uint64_t);
    return be64toh(val);
}

std::string Buffer::read_n_bytes(size_t n) {
    std::string res = {buffer + no_bytes_read, n};
    no_bytes_read += n;
    return res;
}
