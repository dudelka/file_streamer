#pragma once

#include "sender.hpp"
#include "file.hpp"
#include "packet.hpp"
#include "utils.hpp"

#include <cstdint>
#include <memory>

#ifdef CLIENT_MODE
#include <atomic>
#include <queue>

class PacketManager {
public:
    PacketManager(std::shared_ptr<Sender> sender, File file);

    void Run();
    void AckPacket(const Packet& packet, Multithreaded* receiver);
    void AckPacket(Packet&& packet, Multithreaded* receiver);

    uint32_t GetFileChecksum();
    uint32_t GetReceivedChecksum() const;

private:
    std::shared_ptr<Sender> sender_;
    File file_;
    std::queue<Packet> fifo_;
    uint32_t crc32_ {0};
};
#elif SERVER_MODE
#include <unordered_set>

class PacketManager {
public:
    explicit PacketManager(std::shared_ptr<Sender> sender);

    void PushPacket(const Packet& packet);
    void PushPacket(Packet&& packet);

private:
    std::shared_ptr<Sender> sender_;
    File file_;
    std::unordered_set<uint32_t> seq_numbers_;
};
#endif
