#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

struct RelocatedCode {
    std::vector<uint8_t> bytes;
    size_t               insns_size;
    size_t               n_insns;
};

class IRelocator {
public:
    virtual ~IRelocator() = default;
    virtual RelocatedCode relocate(const std::vector<uint8_t>& input_bytes,
                                   size_t                       n_bytes,
                                   uint64_t                     original_ea,
                                   uint64_t                     trampoline_addr,
                                   const std::vector<uint8_t>&  trailer) = 0;

    virtual std::string_view name() const = 0;
};
