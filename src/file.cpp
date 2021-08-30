#include "file.hpp"
#include "utils.hpp"

#include <algorithm>
#include <fstream>
#include <exception>
#include <future>
#include <utility>
#include <cstring>

File::File(const std::string& filename, size_t chunk_size, uint64_t id)
        : id_(id) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can't open file with filename: " + filename + ".");
    }
    file_chunks_ = DivideIntoChunks(file, chunk_size);
    file.close();
}

uint32_t File::GetChecksum() {
    if (crc32_ != 0) {
        return crc32_;
    }
    for (const auto& chunk : file_chunks_) {
        crc32_ = crc32c(crc32_, chunk.data_.data(), chunk.data_.size());
    }
    return crc32_;
}

uint32_t File::GetId() const {
    return id_;
}

std::vector<FileChunk>* File::GetFile() {
    return &file_chunks_;
}

void File::AddChunk(const Packet& packet) {
    AddChunk(std::move(packet));
}

void File::AddChunk(Packet&& packet) {
    if (seq_nums_.count(packet.seq_number_) == 0) {
        FileChunk chunk;
        chunk.seq_num_ = packet.seq_number_;
        chunk.crc32_ = packet.crc32_;
        size_t data_size = packet.size_ - PACKET_HEADER_SIZE;
        chunk.data_.reserve(data_size);
        std::memmove(chunk.data_.data(), packet.data_, data_size);
        file_chunks_.push_back(std::move(chunk));
        seq_nums_.insert(packet.seq_number_);
    }
}

void File::Sort() {
    std::sort(
        file_chunks_.begin(),
        file_chunks_.end(),
        [](const FileChunk& lhs, const FileChunk& rhs) {
            return lhs.seq_num_ < rhs.seq_num_;
        }
    );
}

std::vector<FileChunk> File::DivideIntoChunks(std::ifstream& file, 
        const size_t chunk_size) {
    std::vector<FileChunk> result;
    std::vector<uint8_t> chunk;
    chunk.reserve(chunk_size);
    uint32_t current_chunk = 0;
    uint8_t symbol;
    while (!file.eof()) {
        chunk.clear();
        for (uint16_t i = 0; i < chunk_size; ++i) {
            if (!file.eof()) {
                file >> symbol;
                chunk.push_back(symbol);
            }
        }
        chunk.shrink_to_fit();
        uint32_t crc32 = 0;
        crc32 = crc32c(crc32, chunk.data(), chunk.size());
        result.emplace_back(
            FileChunk{
                .seq_num_ = current_chunk++,
                .crc32_ = crc32,
                .data_ = std::move(chunk)
            }
        );
    }
    return result;
}

std::vector<File> GenerateFiles(const std::vector<std::string>& filenames) {
    std::vector<File> result;
    result.reserve(filenames.size());
    std::vector<std::future<File>> futures;
    futures.reserve(filenames.size());
    uint64_t id = 0;
    for (const auto& filename : filenames) {
        futures.push_back(
            std::async(
                [&filename, &id]() {
                    return File(filename, MAX_PAYLOAD_SIZE, id++);
                }
            )
        );
    }
    for (auto& future : futures) {
        result.push_back(future.get());
    }
    return result;
}
