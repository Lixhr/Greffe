#pragma once

#include <cstdint>
#include <memory>
#include "patch/arch/IArchStubs.hpp"

class PatchLayoutEntry {
    public:
        virtual ~PatchLayoutEntry() = default;

        void        set_offset(uint64_t offset);
        uint64_t    offset() const;

        std::shared_ptr<IArchStubs> stubs;
    protected:
        uint64_t    _offset = 0;
};