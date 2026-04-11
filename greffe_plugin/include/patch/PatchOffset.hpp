#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>

class PatchOffset {
public:
    PatchOffset(int bits, uint64_t patch_base)
        : _value(0)
        , _base(patch_base)
        , _max(bits == 32 ? std::numeric_limits<uint32_t>::max()
                          : std::numeric_limits<uint64_t>::max())
    {}

    PatchOffset& operator=(uint64_t v) {
        _value = v;
        return *this;
    }

    PatchOffset& operator+=(uint64_t rhs) {
        if (rhs > _max - _base - _value)
            throw std::overflow_error(
                "PatchOffset overflow: patch_base + offset exceeds addressable space");
        _value += rhs;
        return *this;
    }

    PatchOffset operator+(uint64_t rhs) const {
        PatchOffset result(*this);
        result += rhs;
        return result;
    }

    operator uint64_t() const { return _value; }

private:
    uint64_t _value;
    uint64_t _base;
    uint64_t _max;
};
