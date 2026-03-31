#pragma once

#include "IArchStubs.hpp"
#include "Target.hpp"
#include "ProjectInfo.hpp"
#include <memory>

class StubsFactory {
public:
    static std::shared_ptr<IArchStubs> create(const Target& t, const ProjectInfo& pi);
};
