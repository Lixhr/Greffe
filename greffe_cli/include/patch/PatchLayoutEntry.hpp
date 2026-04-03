#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "patch/arch/IArchStubs.hpp"

class PatchLayoutEntry {
    public:
        virtual     ~PatchLayoutEntry() = default;

        void                        set_offset(uint64_t offset);
        void                        set_addr(uint64_t addr);
        uint64_t                    offset() const;
        uint64_t                    addr()   const;
        const std::vector<uint8_t>& bytes()  const;
        std::vector<uint8_t>&       bytes();

        std::shared_ptr<IArchStubs> stubs;
    protected:
        uint64_t             _offset = 0;
        uint64_t             _addr   = 0;
        std::vector<uint8_t> _bytes  = {};
};