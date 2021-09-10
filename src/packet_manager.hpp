#pragma once

#include "sender.hpp"
#include "file.hpp"
#include "packet.hpp"
#include "utils.hpp"

#include <cstdint>
#include <memory>
#include <unordered_set>

#ifdef CLIENT_MODE
#include <atomic>
#include <queue>

class PacketManager {
public:
    PacketManager(std::shared_ptr<Sender> sender, File file);

    void Run();
    void AckPacket(const Packet& packet, Multithreaded* receiver);

    uint32_t GetFileChecksum();
    uint32_t GetReceivedChecksum() const;

private:
    std::shared_ptr<Sender> sender_;
    File file_;
    const size_t total_packets_count_ {0};
    std::queue<Packet> fifo_;
    // this set contains sequence numbers of acknowledged packets
    std::unordered_set<uint32_t> acknowledged_packets_;
    uint32_t crc32_ {0};
};
#elif SERVER_MODE
class PacketManager {
public:
    explicit PacketManager(std::shared_ptr<Sender> sender);

    void PushPacket(Packet packet);

private:
    std::shared_ptr<Sender> sender_;
    File file_;
    std::unordered_set<uint32_t> seq_numbers_;
    bool fully_received_ {false};
};
#endif
