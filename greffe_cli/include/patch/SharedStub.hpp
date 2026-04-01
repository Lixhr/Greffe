#pragma once

#include <memory>
#include "IArchStubs.hpp"

class SharedStub {
    public:
        explicit             SharedStub(std::shared_ptr<IArchStubs> stub);
        std::string_view     name() const;
    private:
        uint64_t             _offset;
        std::string_view     _name;
};