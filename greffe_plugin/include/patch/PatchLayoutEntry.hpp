#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "patch/arch/IArchStubs.hpp"

class PatchLayoutEntry {
    public:
        virtual     ~PatchLayoutEntry() = default;

        void                        set_offset(uint64_t offset);
        void                        set_addr(ea_t addr);
        std::vector<uint8_t>&       bytes();
        uint64_t                    offset() const;
        ea_t                        addr()   const;
        const std::vector<uint8_t>& bytes()  const;
        void                        set_color(bgcolor_t color) const;

        std::shared_ptr<IArchStubs> stubs;
    protected:
        uint64_t             _offset = 0;
        ea_t                 _addr   = 0;
        std::vector<uint8_t> _bytes  = {};
};