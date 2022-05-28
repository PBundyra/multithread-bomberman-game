#ifndef BUFFER_H
#define BUFFER_H

#include <string>

#define MAX_DATAGRAM_SIZE 65507
#define BUFFER_SIZE MAX_DATAGRAM_SIZE

// The bytes in the buffer are always in network byte order.
// Reading from buffer returns the bytes in host byte order.
// Using writing functions user must ensure that the bytes are in network byte order.
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

    // Writes a string with given len into the buffer.
    void write_into_buffer(const char *msg, size_t len);

    uint8_t read_1_byte();

    uint16_t read_2_bytes();

    uint32_t read_4_bytes();

    uint64_t read_8_bytes();

    // Reads n bytes from the buffer and returns them as a string
    std::string read_n_bytes(size_t n);
};

#endif //BUFFER_H
