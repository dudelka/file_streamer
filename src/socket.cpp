#include "socket.hpp"

#include <exception>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>

Socket::Socket(const std::string& address, const SocketType type)
        : address_(InitAddress(address))
        , sock_(InitSocket(address_, address, type)) {
    if (type == SocketType::RECEIVER) {
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    }
}

Socket::~Socket() {
    close(sock_);
}

void Socket::SendPacket(Packet* packet_ptr) {
    if (!packet_ptr) {
        throw std::invalid_argument("Pointer to packet can't be nullptr.");
    }
    Packet& packet = *packet_ptr;
    ssize_t ret = sendto(sock_, (void*)&packet, packet.size_, 0, 
        (sockaddr*)&address_, sizeof(address_));
    if (ret != packet.size_) {
        perror("Socket send: ");
        std::string err_msg = 
            "Sending error. Sent bytes doesn't match to bytes to be sent, expected: " 
            + std::to_string(packet.size_) + ", got: " + std::to_string(ret) + ".";
        throw std::runtime_error(err_msg);
    }
}

std::optional<Packet> Socket::ReceivePacket() {
    ssize_t ret = recvfrom(sock_, (void*)&received_packet_, MAX_PACKET_SIZE, 0, nullptr, nullptr);
    if (ret < 0) {
        return std::nullopt;
    }
    if (ret != received_packet_.size_) {
        perror("Socket receive: ");
        throw std::runtime_error("Receiving error. Received bytes doesn't match to bytes to be received.");
    }
    return received_packet_;
}

sockaddr_in Socket::InitAddress(std::string_view address) {
    sockaddr_in result;
    std::memset((char*)&result, 0, sizeof(result));
    size_t pos = address.find(':');
    if (pos == address.npos) {
        throw std::invalid_argument("Wrong address format: " 
            + std::string(address) + ". Expected ':' symbol.");
    }
    result.sin_family = AF_INET;
    result.sin_port = htons(std::stoi(std::string(address.substr(pos + 1))));
    int pton = inet_pton(AF_INET, address.substr(0, pos).data(), &result.sin_addr);
    if (pton < 0) {
        perror("Socket inet_pton: ");
        throw std::invalid_argument("Wrong address format. Expected: XXX.YYY.ZZZ.KKK:PORT. Got: "
            + std::string(address) + ".");
    }
    return result;
}

int Socket::InitSocket(const sockaddr_in& address, const std::string& address_str, 
        const SocketType type) {
    int result = socket(AF_INET, SOCK_DGRAM, 0);
    if (result < 0) {
        perror("Socket create: ");
        throw std::runtime_error("Can't create socket.");
    }
    if (type == SocketType::RECEIVER) {
        perror("Socket bind: ");
        if (bind(result, (sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Can't bind socket on address " + address_str + ".");
        }
    }
    return result;
}
