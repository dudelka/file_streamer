#include "receiver.hpp"

#include <iostream>
#include <thread>
#include <functional>

#ifdef CLIENT_MODE
Receiver::Receiver(const std::string& receive_address,
            std::shared_ptr<Sender> sender, std::vector<File> files) 
        : sock_(receive_address, SocketType::RECEIVER) {
    for (auto& file : files) {
        packet_managers_.emplace(
            std::make_pair(file.GetId(), PacketManager(sender, std::move(file)))
        );
    }
}

void Receiver::Connect(std::shared_ptr<Sender> sender) {
    std::cerr << "[Receiver] Establishing connection with server..." << std::endl;
    ReceiveControlPackets(sender, std::mem_fn(&Sender::Connect), 
        PacketType::CONNECT_RESPONSE);
    std::cerr << "[Receiver] Connection with server has been established." << std::endl;
}

void Receiver::Shutdown(std::shared_ptr<Sender> sender) {
    std::cerr << "[Receiver] Sending shutdown request to server..." << std::endl;
    ReceiveControlPackets(sender, std::mem_fn(&Sender::Shutdown), 
        PacketType::SHUTDOWN_RESPONSE);
    std::cerr << "[Receiver] Server received shutdown request." << std::endl;
}

void Receiver::Run() {
    std::unordered_map<uint32_t, std::thread> managers_threads;
    for (auto& [id, packet_manager] : packet_managers_) {
        managers_threads[id] = std::thread{&PacketManager::Run, &packet_manager};
    }
    while (!should_stop_) {
        std::optional<Packet> packet = sock_.ReceivePacket();
        if (packet == std::nullopt) {
            continue;
        }
        uint32_t id = GetPacketId(*packet);
        if (packet_managers_.count(id) == 0) {
            std::cerr << "[Receiver] Received packet with id that wasn't sent to server." 
                << std::endl;
            continue;
        }
        packet_managers_.at(id).AckPacket(*packet, this);
    }
    for (auto& [id, manager_thread] : managers_threads) {
        manager_thread.join();
    }
}

void Receiver::Stop() {
    stopped_managers_count_ += 1;
    if (stopped_managers_count_ == packet_managers_.size()) {
        std::cerr << "[Receiver] Stop signal received. Stopping receiver..." << std::endl;
        should_stop_ = true;
    }
}

std::vector<std::pair<uint32_t, uint32_t>> Receiver::GetSentChecksums() {
    std::vector<std::pair<uint32_t, uint32_t>> result;
    result.reserve(packet_managers_.size());
    for (auto& [id, packet_manager] : packet_managers_) {
        result.emplace_back(std::make_pair(id, packet_manager.GetFileChecksum()));
    }
    return result;
}

std::vector<std::pair<uint32_t, uint32_t>> Receiver::GetReceivedChecksums() const {
    std::vector<std::pair<uint32_t, uint32_t>> result;
    result.reserve(packet_managers_.size());
    for (auto& [id, packet_manager] : packet_managers_) {
        result.emplace_back(std::make_pair(id, packet_manager.GetReceivedChecksum()));
    }
    return result;
}
#elif SERVER_MODE
Receiver::Receiver(const std::string& receive_address, 
        std::shared_ptr<Sender> sender, uint32_t timeout)
    : sock_(receive_address, SocketType::RECEIVER) 
    , sender_(sender) 
    , timeout_(timeout) {
}

void Receiver::Run() {
    Timer timer(this, timeout_);
    std::thread timer_thread{&Timer::Run, &timer};
    while (!should_stop_) {
        std::optional<Packet> packet = sock_.ReceivePacket();
        if (packet == std::nullopt) {
            continue;
        }
        timer.NotifyHasInput();
        switch (static_cast<PacketType>(packet->type_)) {
            case PacketType::CONNECT : {
                std::cerr << "[Receiver] Processing connect request from client." 
                    << std::endl;
                Packet response;
                response.size_ = PACKET_HEADER_SIZE;
                response.type_ = static_cast<uint8_t>(PacketType::CONNECT_RESPONSE);
                sender_->ProcessPacket(std::move(response));
                break;
            } case PacketType::PUT : {
                uint32_t id = GetPacketId(*packet);
                if (packet_managers_.count(id) == 0) {
                    packet_managers_.emplace(std::make_pair(id, PacketManager(sender_)));
                }
                packet_managers_.at(id).PushPacket(std::move(*packet));
                break;
            } case PacketType::SHUTDOWN : {
                std::cerr << "[Receiver] Processing shutdown request from client." 
                    << std::endl;
                Packet response;
                response.size_ = PACKET_HEADER_SIZE;
                response.type_ = static_cast<uint8_t>(PacketType::SHUTDOWN_RESPONSE);
                sender_->ProcessPacket(std::move(response));
                break;
            } default :
                std::cerr << "Unsupported packet type for server. Skip it." << std::endl;
                break;
        }
    }
    timer.Stop();
    timer_thread.join();
}
#endif
