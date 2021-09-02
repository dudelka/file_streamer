#pragma once

#include <chrono>
#include <cstdint>
#include <cstddef>

const size_t MAX_PACKET_SIZE {1472};
const size_t PACKET_HEADER_SIZE {33};
const size_t MAX_PAYLOAD_SIZE {MAX_PACKET_SIZE - PACKET_HEADER_SIZE};

enum class PacketType {
    CONNECT,
    CONNECT_RESPONSE,
    ACK,
    PUT,
    SHUTDOWN,
    SHUTDOWN_RESPONSE,
};

struct Packet {
    uint32_t seq_number_ {0};
    uint32_t seq_total_ {0};
    // uint64_t time_ {0};
    std::chrono::time_point<std::chrono::steady_clock> time_;
    uint32_t size_ {0};
    uint32_t crc32_ {0};
    uint8_t type_;
    uint8_t id_[8]{};
    uint8_t data_[MAX_PAYLOAD_SIZE]{};
};
