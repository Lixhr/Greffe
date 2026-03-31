#pragma once

#include "IArchStubs.hpp"
#include <memory>
#include <string_view>

class StubsFactory {
public:
    static std::shared_ptr<IArchStubs> create(int bits, std::string_view mode);
};
