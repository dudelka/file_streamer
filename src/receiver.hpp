#pragma once

#include "utils.hpp"
#include "sender.hpp"
#include "socket.hpp"
#include "packet_manager.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <string>

#ifdef CLIENT_MODE
#include "file.hpp"
#include "packet.hpp"

#include <vector>
#include <utility>
#include <optional>

class Receiver : public Multithreaded {
public:
    Receiver(const std::string& receive_address,
        std::shared_ptr<Sender> sender, std::vector<File> files);

    void Connect(std::shared_ptr<Sender> sender);
    void Shutdown(std::shared_ptr<Sender> sender);

    void Run() override;
    void Stop() override;

    // pair file_id -> file_checksum
    std::vector<std::pair<uint32_t, uint32_t>> GetSentChecksums();
    std::vector<std::pair<uint32_t, uint32_t>> GetReceivedChecksums() const;

private:
    Socket sock_;
    std::unordered_map<uint32_t, PacketManager> packet_managers_; // file id -> PacketManager

    size_t stopped_managers_count_ {0};

    template <typename SendControlPacketsFunc>
    void ReceiveControlPackets(std::shared_ptr<Sender> sender, 
            SendControlPacketsFunc control_sender, const PacketType packet_type) {
        while (true) {
            control_sender(*sender);
            std::optional<Packet> packet = sock_.ReceivePacket();
            if (packet == std::nullopt) {
                continue;
            }
            if (static_cast<PacketType>(packet->type_) ==
                    packet_type) {
                break;
            }
        }
    }
};
#elif SERVER_MODE
class Receiver : public Multithreaded {
public:
    Receiver(const std::string& receive_address, 
        std::shared_ptr<Sender> sender, uint32_t timeout);

    void Run() override;

private:
    Socket sock_;
    std::shared_ptr<Sender> sender_;
    uint32_t timeout_;
    std::unordered_map<uint32_t, PacketManager> packet_managers_; // file id -> PacketManager
};
#endif
