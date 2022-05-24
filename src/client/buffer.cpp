#include "buffer.h"

void Buffer::write_into_buffer(uint8_t msg) {
    auto val = msg;
    memcpy(buffer + no_bytes, &val, sizeof(uint8_t));
    no_bytes += sizeof(uint8_t);
}

void Buffer::write_into_buffer(uint16_t msg) {
    auto val = htobe16(msg);
    memcpy(buffer + no_bytes, &val, sizeof(uint16_t));
    no_bytes += sizeof(uint16_t);
}

void Buffer::write_into_buffer(uint32_t msg) {
    auto val = htobe32(msg);
    memcpy(buffer + no_bytes, &val, sizeof(uint32_t));
    no_bytes += sizeof(uint32_t);
}

void Buffer::write_into_buffer(uint64_t msg) {
    auto val = htobe64(msg);
    memcpy(buffer + no_bytes, &val, sizeof(uint64_t));
    no_bytes += sizeof(uint64_t);
}

void Buffer::write_str_into_buffer(std::string &msg) {
    memcpy(buffer + no_bytes, msg.c_str(), msg.size());
    no_bytes += msg.size();
}

size_t Buffer::get_no_bytes() const {
    return no_bytes;
}

std::string Buffer::cpy_buffer() {
    return {buffer, no_bytes};
}

uint8_t Buffer::read_1_bytes() {
    uint8_t val;
    no_bytes -= sizeof(uint8_t);
    memcpy(&val, buffer + no_bytes, sizeof(uint8_t));
    return val;
}

uint16_t Buffer::read_2_bytes() {
    uint16_t val;
    no_bytes -= sizeof(uint16_t);
    memcpy(&val, buffer + no_bytes, sizeof(uint16_t));
    return be16toh(val);
}

uint32_t Buffer::read_4_bytes() {
    uint32_t val;
    no_bytes -= sizeof(uint32_t);
    memcpy(&val, buffer + no_bytes, sizeof(uint32_t));
    return be32toh(val);
}

uint64_t Buffer::read_8_bytes() {
    uint64_t val;
    no_bytes -= sizeof(uint64_t);
    memcpy(&val, buffer + no_bytes, sizeof(uint64_t));
    return be64toh(val);
}

std::string Buffer::read_str_from_buffer() {
    return {buffer, no_bytes};
}
