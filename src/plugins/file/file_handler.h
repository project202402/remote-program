#pragma once

#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>

class FileHandler {
public:
    explicit FileHandler(const std::string& filePath, std::ios::openmode mode)
        : filePath_(filePath), file_(filePath, mode) {
        if (!file_.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }
    }

    std::streamsize size() const {
        return fileSize_;
    }

    std::streamsize offset() const {
        return offset_;
    }

protected:
    std::string filePath_;
    std::fstream file_;
    std::streamsize offset_{};  // Tracks the current offset in the file
    std::streamsize fileSize_{};

    void calculateFileSize() {
        file_.seekg(0, std::ios::end);
        fileSize_ = file_.tellg();
        file_.seekg(0, std::ios::beg);  // Reset back to the beginning of the file
    }
};

class FileDownloader : public FileHandler {
public:
    explicit FileDownloader(const std::string& target, std::streamsize size)
        : FileHandler(target, std::ios::binary | std::ios::out | std::ios::app), size_(size) {
        file_.seekp(0, std::ios::end);
        offset_ = file_.tellp();
    }

    void writeChunk(const std::vector<char>& chunk) {
        if (file_) {
            file_.write(chunk.data(), chunk.size());
            offset_ += chunk.size();
        }
    }

private:
    std::streamsize size_;
};

class FileUploader : public FileHandler {
public:
    explicit FileUploader(const std::string& source, std::size_t chunkSize)
        : FileHandler(source, std::ios::binary | std::ios::in ), chunkSize_(chunkSize) {
        calculateFileSize();
    }

    std::vector<char> readChunk() {
        if (offset_ >= fileSize_) {
            return {};
        }
        std::vector<char> buffer(chunkSize_);
        file_.read(buffer.data(), buffer.size());
        buffer.resize(file_.gcount());  // Resize buffer to actual data read
        offset_ += buffer.size();
        return buffer;
    }

private:
    std::size_t chunkSize_;
};