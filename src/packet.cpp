#include "packet.hpp"

#include <cstring>
#include <utility>

#include <arpa/inet.h>

void Packet::Serialize() {
    seq_number_ = htonl(seq_number_);
    seq_total_ = htonl(seq_total_);
    size_ = htonl(size_);
    crc32_ = htonl(crc32_);
    type_ = htons(seq_total_);
}

void Packet::Deserialize() {
    seq_number_ = ntohl(seq_number_);
    seq_total_ = ntohl(seq_total_);
    size_ = ntohl(size_);
    crc32_ = ntohl(crc32_);
    type_ = ntohs(type_);
}

uint32_t GetPacketId(const Packet& packet) {
    uint32_t result = 0;
    std::memcpy(&result, packet.id_, sizeof(result));
    return result;
}
