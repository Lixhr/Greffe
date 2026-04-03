#pragma once

#include <memory>
#include "IArchStubs.hpp"
#include "PatchLayoutEntry.hpp"

class SharedStub : public PatchLayoutEntry {
    public:
        SharedStub(std::shared_ptr<IArchStubs> s, 
                   uint64_t initial_offset,
                   uint64_t addr);
        std::string_view            name() const;
        uint64_t                    end()  const;
        const std::vector<uint8_t>  bytes() const;

    private:
        std::vector<uint8_t>        _bytes;
        std::string_view            _name;
};