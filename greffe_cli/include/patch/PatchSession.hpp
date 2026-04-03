#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

class PatchSession {
    public:
        PatchSession(const std::filesystem::path& bin_path, uint64_t bin_base);

        void    patch(uint64_t address, const std::vector<uint8_t>& bytes);
        void    save(const std::filesystem::path& outfile) const;

    private:
        std::vector<uint8_t> _buffer;
        uint64_t             _bin_base;
};