#include "sender.hpp"

#include <utility>

#ifdef CLIENT_MODE
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

Sender::Sender(const std::string& send_address, uint32_t resend_timeout) 
    : sock_(send_address, SocketType::SENDER)
    , resend_timeout_(resend_timeout) {
}

void Sender::Connect() {
    SendControlPacket(PacketType::CONNECT);
}

void Sender::Shutdown() {
    SendControlPacket(PacketType::SHUTDOWN);
}

void Sender::PushPacket(const Packet& packet) {
    PushPacket(std::move(packet));
}

void Sender::PushPacket(Packet&& packet) {
    std::unique_lock<std::mutex> lock(fifo_lock_.m_);
    fifo_.push(std::move(packet));
    has_packets_ = true;
    fifo_lock_.cv_.notify_one();
}

void Sender::Run() {
    using namespace std::chrono;
    std::thread resend_thread([this]{this->ResendPackets();});
    while (!should_stop_) {
        std::unique_lock<std::mutex> lock(fifo_lock_.m_);
        while (!has_packets_ && !should_stop_) {
            fifo_lock_.cv_.wait_for(lock, microseconds{resend_timeout_});
        }
        if (should_stop_) {
            std::cerr << "[Sender] Stop signal received. Stopping sender..." 
                << std::endl;
            break;
        }
        Packet packet = std::move(fifo_.front());
        fifo_.pop();
        if (fifo_.empty()) {
            has_packets_ = false;
        }
        auto now = steady_clock::now();
        packet.time_ = now;
        sock_.SendPacket(packet);
        std::unique_lock<std::mutex> ack_lock(ack_lock_.m_);
        not_ack_packets_.push_back(std::move(packet));
        has_not_ack_packets_ = true;
        ack_lock_.cv_.notify_one();
    }
    resend_thread.join();
}

void Sender::AckPacket(uint32_t id, uint32_t packet_num) {
    std::unique_lock<std::mutex> ack_lock(ack_lock_.m_);
    auto it = std::find_if(
        not_ack_packets_.begin(), 
        not_ack_packets_.end(), 
        [id, packet_num](const Packet& packet) {
            return GetPacketId(packet) == id && packet.seq_number_ == packet_num;
        }
    );
    if (it != not_ack_packets_.end()) {
        not_ack_packets_.erase(it);
    }
    if (not_ack_packets_.empty()) {
        has_not_ack_packets_ = false;
    }
}

void Sender::SendControlPacket(const PacketType packet_type) {
   Packet packet;
   packet.size_ = PACKET_HEADER_SIZE;
   packet.type_ = static_cast<uint8_t>(packet_type);
   sock_.SendPacket(packet);
}

void Sender::ResendPackets() {
    using namespace std::chrono;
    while (!should_stop_) {
        std::unique_lock<std::mutex> lock(ack_lock_.m_);
        while (!has_not_ack_packets_ && !should_stop_) {
            ack_lock_.cv_.wait_for(lock, microseconds(resend_timeout_));
        }
        if (should_stop_) {
            break;
        }
        auto now = steady_clock::now();
        auto& packet = not_ack_packets_.front();
        if (duration_cast<microseconds>(now - packet.time_).count() < resend_timeout_) {
            continue;
        }
        sock_.SendPacket(packet);
        not_ack_packets_.push_back(std::move(packet));
        not_ack_packets_.pop_front();
    }
}
#elif SERVER_MODE
Sender::Sender(const std::string& send_address) 
    : sock_(send_address, SocketType::SENDER) {
}

void Sender::ProcessPacket(const Packet& packet) {
    ProcessPacket(std::move(packet));
}

void Sender::ProcessPacket(Packet&& packet) {
    sock_.SendPacket(std::move(packet));
}
#endif
