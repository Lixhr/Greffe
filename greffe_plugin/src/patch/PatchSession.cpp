#include "PatchSession.hpp"
#include <fstream>
#include <stdexcept>

PatchSession::PatchSession(const std::filesystem::path& bin_path, ea_t bin_base)
    : _bin_base(bin_base) {
    std::ifstream src(bin_path, std::ios::binary);
    if (!src)
        throw std::runtime_error("PatchSession: cannot open " + bin_path.string());

    _buffer.assign(std::istreambuf_iterator<char>(src),
                   std::istreambuf_iterator<char>{});
}

void PatchSession::patch(ea_t address, const std::vector<uint8_t>& bytes) {
    if (bytes.empty())
        return;

    uint64_t write_offset = address - _bin_base;
    uint64_t required_size = write_offset + bytes.size();
    if (required_size > _buffer.size())
        _buffer.resize(required_size, 0);

    std::copy(bytes.begin(), bytes.end(), _buffer.begin() + write_offset);
}

void PatchSession::save(const std::filesystem::path& outfile) const {
    std::ofstream dst(outfile, std::ios::binary | std::ios::trunc);
    if (!dst)
        throw std::runtime_error("PatchSession: cannot write " + outfile.string());

    dst.write(reinterpret_cast<const char*>(_buffer.data()),
              static_cast<std::streamsize>(_buffer.size()));
    if (!dst)
        throw std::runtime_error("PatchSession: write failed for " + outfile.string());
}