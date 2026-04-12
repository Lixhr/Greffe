#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include "ida.hpp"

class PatchSession {
    public:
        PatchSession(const std::filesystem::path& bin_path, ea_t bin_base);

        void    patch(ea_t address, const std::vector<uint8_t>& bytes);
        void    save(const std::filesystem::path& outfile) const;

    private:
        std::vector<uint8_t> _buffer;
        uint64_t             _bin_base;
};