#include "BinaryPatcher.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

void BinaryPatcher::patch(const std::filesystem::path& bin_path,
                          uint64_t                     write_offset,
                          const std::vector<uint8_t>&  bytes)
{
    if (bytes.empty())
        return;

    std::fstream f(bin_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!f)
        throw std::runtime_error("BinaryPatcher: cannot open " + bin_path.string());

    f.seekg(0, std::ios::end);
    uint64_t file_size = static_cast<uint64_t>(f.tellg());

    if (write_offset > file_size) {
        f.seekp(0, std::ios::end);
        std::vector<uint8_t> padding(write_offset - file_size, 0);
        f.write(reinterpret_cast<const char*>(padding.data()),
                static_cast<std::streamsize>(padding.size()));
        if (!f)
            throw std::runtime_error("BinaryPatcher: failed to zero-extend file");
    }

    f.seekp(static_cast<std::streamoff>(write_offset), std::ios::beg);
    f.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    if (!f)
        throw std::runtime_error("BinaryPatcher: write failed at offset 0x"
                                 + [&]{ char buf[17]; snprintf(buf, sizeof(buf), "%lx", write_offset);
                                        return std::string(buf); }());
}
