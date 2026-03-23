#pragma once

#include "IRelocator.hpp"
#include "Target.hpp"
#include "ProjectInfo.hpp"
#include <memory>

class RelocatorFactory {
public:
    static std::unique_ptr<IRelocator> create(const Target& t, const ProjectInfo& pi);
};
