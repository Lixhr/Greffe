#pragma once

#include <cstddef>
#include <cstdint>

class HandlerBin {
public:
    HandlerBin() = default;

    size_t   size()                            const { return 0; }
    uint64_t handler_addr(size_t, uint64_t base) const { return base; }
};
