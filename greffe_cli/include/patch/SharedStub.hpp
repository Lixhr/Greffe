#pragma once

#include <memory>
#include "IArchStubs.hpp"
#include "PatchLayoutEntry.hpp"

class SharedStub : public PatchLayoutEntry {
    public:
        explicit             SharedStub(std::shared_ptr<IArchStubs> stub);
        std::string_view     name() const;
    private:
        std::string_view     _name;
};