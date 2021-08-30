#pragma once

#include "packet.hpp"

#include <cstdint>
#include <unordered_set>
#include <vector>
#include <string>

struct FileChunk {
    uint32_t seq_num_;
    uint32_t crc32_;
    std::vector<uint8_t> data_;
};

class File {
public:
    File() = default;
    File(const std::string& filename, size_t chunk_size, uint64_t id);

    uint32_t GetChecksum();
    uint32_t GetId() const;
    std::vector<FileChunk>* GetFile();

    void AddChunk(const Packet& packet);
    void AddChunk(Packet&& packet);
    void Sort();

private:
    uint64_t id_ {0};
    uint32_t crc32_ {0};
    std::vector<FileChunk> file_chunks_;
    // contains seq_nums of chunks that file_chunks_ has
    // only needed for receiver to drop repeated packets
    std::unordered_set<uint32_t> seq_nums_;

    std::vector<FileChunk> DivideIntoChunks(std::ifstream& file, 
        const size_t chunk_size);
};

std::vector<File> GenerateFiles(const std::vector<std::string>& filenames);
