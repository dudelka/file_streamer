#pragma once

#include "packet.hpp"

#include <optional>
#include <string>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

enum class SocketType {
    SENDER,
    RECEIVER,
};

class Socket {
public:
    Socket(const std::string& address,
        const SocketType type);

    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;

    ~Socket();

    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;

    void SendPacket(const Packet& packet);
    void SendPacket(Packet&& packet);

    std::optional<Packet> ReceivePacket();

private:
    int sock_;
    sockaddr_in address_;

    Packet received_packet_;

    sockaddr_in InitAddress(std::string_view address);
    int InitSocket(const std::string& address, const SocketType type);
};
