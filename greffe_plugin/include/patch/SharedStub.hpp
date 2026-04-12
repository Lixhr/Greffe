#pragma once

#include <memory>
#include "IArchStubs.hpp"
#include "PatchLayoutEntry.hpp"

class SharedStub : public PatchLayoutEntry {
    public:
        SharedStub(std::shared_ptr<IArchStubs> s, ea_t addr);
        std::string_view name() const;

    private:
        std::string_view _name;
};
