#include "packet_manager.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <cstring>

#ifdef CLIENT_MODE
PacketManager::PacketManager(std::shared_ptr<Sender> sender, File file)
    : sender_(sender) 
    , file_(std::move(file)) {
}

void PacketManager::Run() {
    std::vector<FileChunk>& file = *file_.GetFile();
    Packet packet;
    uint32_t seq_num = 0;
    for (auto& file_chunk : file) {
        packet.seq_number_ = ++seq_num;
        packet.seq_total_ = file.size();
        packet.size_ = PACKET_HEADER_SIZE + file_chunk.data_.size();
        packet.crc32_ = file_chunk.crc32_;
        packet.type_ = static_cast<uint8_t>(PacketType::PUT);
        uint32_t id = file_.GetId();
        std::memcpy(packet.id_, &id, sizeof(id));
        std::memmove(packet.data_, file_chunk.data_.data(), file_chunk.data_.size());
        sender_->PushPacket(std::move(packet));
    }
}

void PacketManager::AckPacket(const Packet& packet, Multithreaded* receiver) {
    if (!receiver) {
        throw std::invalid_argument("Pointer to Receiver should not be nullptr.");
    }
    if (static_cast<PacketType>(packet.type_) == PacketType::ACK) {
        sender_->AckPacket(GetPacketId(packet), packet.seq_number_);
    }
    if (packet.seq_number_ == packet.seq_total_) {
        std::cerr << "[PacketManager] File with id = " << GetPacketId(packet) 
            << " was fully sent to server." << std::endl;
        crc32_ = packet.crc32_;
        receiver->Stop();
    }
}

uint32_t PacketManager::GetFileChecksum() {
    return file_.GetChecksum();
}

uint32_t PacketManager::GetReceivedChecksum() const {
    return crc32_;
}
#elif SERVER_MODE
PacketManager::PacketManager(std::shared_ptr<Sender> sender) 
    : sender_(sender) {
}

void PacketManager::PushPacket(Packet packet) {
    if (seq_numbers_.count(packet.seq_number_) == 0) {
        size_t packet_size = packet.size_ - PACKET_HEADER_SIZE;
        uint32_t checksum = 0;
        checksum = crc32c(checksum, packet.data_, packet_size);
        if (checksum != packet.crc32_) {
            std::cerr << "[PacketManager] Received damaged packet with id = " 
                << GetPacketId(packet) << " and seq_number = " 
                << packet.seq_number_ << "." << std::endl;
        }
        FileChunk chunk;
        chunk.seq_num_ = packet.seq_number_;
        chunk.crc32_ = packet.crc32_;
        chunk.data_.resize(packet_size);
        std::memmove(chunk.data_.data(), packet.data_, packet_size);
        file_.AddChunk(std::move(chunk));
        seq_numbers_.insert(packet.seq_number_);
    }
    Packet ack_packet;
    ack_packet.seq_number_ = packet.seq_number_;
    ack_packet.seq_total_ = seq_numbers_.size();
    ack_packet.size_ = PACKET_HEADER_SIZE;
    if (seq_numbers_.size() == packet.seq_total_) {
        std::cerr << "[PacketManager] Fully received file with id = " 
            << GetPacketId(packet) << "." << std::endl;
        file_.Sort();
        ack_packet.crc32_ = file_.GetChecksum();
    }
    ack_packet.type_ = static_cast<uint8_t>(PacketType::ACK);
    std::memmove(ack_packet.id_, packet.id_, sizeof(packet.id_));
    sender_->ProcessPacket(std::move(ack_packet));
}
#endif
