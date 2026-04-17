#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "patch/arch/IArchStubs.hpp"

enum PLEType {
    entry_shstub,
    entry_plan,
    entry_branch,
    entry_handlerbin
};

class PatchLayoutEntry {
    public:
        virtual     ~PatchLayoutEntry() = default;

        void                        set_offset(uint64_t offset);
        void                        set_addr(ea_t addr);
        std::vector<uint8_t>&       bytes();
        uint64_t                    offset() const;
        ea_t                        addr()   const;
        const std::vector<uint8_t>& bytes()  const;
        PLEType                     type()   const;
        void                        free();

        std::shared_ptr<IArchStubs> stubs;
    protected:
        explicit PatchLayoutEntry(PLEType t) : _type(t) {}

        uint64_t             _offset = 0;
        ea_t                 _addr   = 0;
        std::vector<uint8_t> _bytes  = {};
        PLEType              _type;
};