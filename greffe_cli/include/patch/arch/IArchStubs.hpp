#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

class IArchStubs {
public:
    virtual ~IArchStubs() = default;

    virtual std::vector<uint8_t> save_ctx   (uint64_t at) = 0;
    virtual std::vector<uint8_t> restore_ctx(uint64_t at) = 0;
    virtual std::vector<uint8_t> branch(uint64_t from, uint64_t to) = 0;
    virtual std::vector<uint8_t> call  (uint64_t from, uint64_t to) = 0;
    virtual size_t               branch_placeholder_size() const = 0;
    virtual std::string_view name() const = 0;
};
