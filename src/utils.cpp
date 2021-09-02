#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <thread>
#include <cstring>

void Multithreaded::Stop() {
    should_stop_ = true;
}

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len) {
    int k;
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
        crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}

uint32_t GetPacketId(const Packet& packet) {
    uint32_t result = 0;
    std::memcpy(&result, packet.id_, sizeof(result));
    return result;
}

#ifdef CLIENT_MODE
// sorts vector that contains pairs file_id -> file_checksum
void Sort(std::vector<std::pair<uint32_t, uint32_t>>& to_sort) {
    std::sort(
        to_sort.begin(),
        to_sort.end(),
        [](const std::pair<uint32_t, uint32_t>& lhs,
                const std::pair<uint32_t, uint32_t>& rhs) {
            return lhs.first < rhs.first;
        }
    );
}
#endif

#ifdef SERVER_MODE
Timer::Timer(Multithreaded* multithreaded, uint32_t timeout) 
        : multithreaded_(multithreaded)
        , timeout_(timeout) {
    if (!multithreaded_) {
        throw std::invalid_argument("[Timer] Pointer to Multithreaded can't be nullptr");
    }
}

void Timer::Run() {
    while (!should_stop_) {
        std::this_thread::sleep_for(std::chrono::seconds{timeout_});
        if (!has_input_) {
            std::cerr << "No input in last " << timeout_ << " seconds. Stopping receiver..." 
                << std::endl;
            multithreaded_->Stop();
            break;
        }
        has_input_ = false;
    }
}

void Timer::NotifyHasInput() {
    has_input_ = true;
}
#endif
