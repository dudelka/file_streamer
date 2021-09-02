#pragma once

#include "socket.hpp"
#include "packet.hpp"

#include <string>

#ifdef CLIENT_MODE
#include "utils.hpp"

#include <cstdint>
#include <atomic>
#include <queue>
#include <list>

class Sender : public Multithreaded {
public:
    Sender(const std::string& send_address, uint32_t resend_timeout);

    void Connect();
    void Shutdown();

    void PushPacket(Packet packet);

    virtual void Run() override;

    void AckPacket(uint32_t id, uint32_t packet_num);

private:
    Socket sock_;
    uint32_t resend_timeout_;

    std::queue<Packet> fifo_;
    std::list<Packet> not_ack_packets_;

    std::atomic<bool> has_packets_ {false};
    std::atomic<bool> has_not_ack_packets_ {false};
    LockPrimitives fifo_lock_;
    LockPrimitives ack_lock_;

    void SendControlPacket(const PacketType packet_type);
    void ResendPackets();
};
#elif SERVER_MODE
class Sender {
public:
    explicit Sender(const std::string& send_address);

    void ProcessPacket(const Packet& packet);
    void ProcessPacket(Packet&& packet);

private:
    Socket sock_;
};
#endif
