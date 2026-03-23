#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

class BinaryPatcher {
public:
    static void patch(const std::filesystem::path& bin_path,
                      uint64_t                     write_offset,
                      const std::vector<uint8_t>&  bytes);
};
