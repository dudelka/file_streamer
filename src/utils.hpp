#pragma once

#include "packet.hpp"

#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

struct LockPrimitives {
    std::mutex m_;
    std::condition_variable cv_;
};

class Multithreaded {
public:
    virtual ~Multithreaded() = default;

    virtual void Run() = 0;
    virtual void Stop();

protected:
    std::atomic<bool> should_stop_ {false};
};

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);

uint32_t GetPacketId(const Packet& packet);

#ifdef CLIENT_MODE
#include <vector>
#include <utility>

// sorts vector that contains pairs file_id -> file_checksum
void Sort(std::vector<std::pair<uint32_t, uint32_t>>& to_sort);
#endif

#ifdef SERVER_MODE
class Timer : public Multithreaded {
public:
    Timer(Multithreaded* multithreaded, const uint32_t timeout);

    virtual void Run() override;
    void NotifyHasInput();

private:
    Multithreaded* multithreaded_;
    uint32_t timeout_;
    std::atomic<bool> has_input_ {false};
};
#endif
