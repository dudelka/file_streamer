#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <thread>

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

void Multithreaded::Stop() {
    should_stop_ = true;
}

#ifdef SERVER_MODE
Timer::Timer(std::shared_ptr<Multithreaded> multithreaded, uint32_t timeout) 
    : multithreaded_(multithreaded)
    , timeout_(timeout) {
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
