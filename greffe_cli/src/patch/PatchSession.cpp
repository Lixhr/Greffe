#include "PatchSession.hpp"
#include <fstream>
#include <stdexcept>

PatchSession::PatchSession(const std::filesystem::path& bin_path, uint64_t bin_base)
    : bin_base_(bin_base) {
    std::ifstream src(bin_path, std::ios::binary);
    if (!src)
        throw std::runtime_error("PatchSession: cannot open " + bin_path.string());

    buffer_.assign(std::istreambuf_iterator<char>(src),
                   std::istreambuf_iterator<char>{});
}

void PatchSession::patch(uint64_t address, const std::vector<uint8_t>& bytes) {
    if (bytes.empty())
        return;

    uint64_t write_offset = address - bin_base_;
    uint64_t required_size = write_offset + bytes.size();
    if (required_size > buffer_.size())
        buffer_.resize(required_size, 0);

    std::copy(bytes.begin(), bytes.end(), buffer_.begin() + write_offset);
}

void PatchSession::save(const std::filesystem::path& outfile) const {
    std::ofstream dst(outfile, std::ios::binary | std::ios::trunc);
    if (!dst)
        throw std::runtime_error("PatchSession: cannot write " + outfile.string());

    dst.write(reinterpret_cast<const char*>(buffer_.data()),
              static_cast<std::streamsize>(buffer_.size()));
    if (!dst)
        throw std::runtime_error("PatchSession: write failed for " + outfile.string());
}