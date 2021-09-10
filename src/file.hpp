#pragma once

#include "packet.hpp"

#include <cstdint>
#include <vector>
#include <string>

struct FileChunk {
    uint32_t seq_num_ {0};
    uint32_t crc32_ {0};
    std::vector<uint8_t> data_;
};

class File {
public:
    File() = default;
    File(const std::string& filename, size_t chunk_size, uint64_t id);

    uint32_t GetChecksum();
    uint32_t GetId() const;
    std::vector<FileChunk>* GetFile();

    void AddChunk(FileChunk chunk);
    void Sort();

private:
    uint64_t id_ {0};
    uint32_t crc32_ {0};
    std::vector<FileChunk> file_chunks_;

    std::vector<FileChunk> DivideIntoChunks(std::ifstream& file, 
        const size_t chunk_size);
};

std::vector<File> GenerateFiles(const std::vector<std::string>& filenames);
