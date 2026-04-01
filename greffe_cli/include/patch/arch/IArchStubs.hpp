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
        virtual std::string_view     name() const = 0;
        virtual uint8_t              alignment() const = 0;

        uint64_t align_offset(uint64_t offset) const {
            const uint8_t a = alignment();
            return (offset + a - 1) & ~static_cast<uint64_t>(a - 1);
        }
};
