#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "patch/arch/IArchStubs.hpp"


// The order is important here, because the gui are applied by type's order
enum PLEType {
    entry_shstub,
    entry_branch,
    entry_handlerbin,
    entry_plan
};


class PatchLayoutEntry {
    public:
        virtual     ~PatchLayoutEntry() = default;

        void                        set_offset(uint64_t offset);
        void                        set_addr(ea_t addr);
        std::vector<uint8_t>&       bytes();
        uint64_t                    offset() const;
        ea_t                        ea()   const;
        ea_t                        end_ea()   const;
        const std::vector<uint8_t>& bytes()  const;
        PLEType                     type()   const;

        std::shared_ptr<IArchStubs> stubs;
    protected:
        explicit PatchLayoutEntry(PLEType t) : _type(t) {}

        uint64_t             _offset = 0;
        ea_t                 _addr   = 0;
        std::vector<uint8_t> _bytes  = {};
        PLEType              _type;
};

typedef std::unique_ptr<PatchLayoutEntry> unique_ple_t;
