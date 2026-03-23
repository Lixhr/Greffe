#pragma once

#include "Target.hpp"
#include "arch/IArchStubs.hpp"
#include "arch/IRelocator.hpp"
#include <cstdint>
#include <vector>

namespace TrampolineBuilder {

    std::vector<uint8_t> build(const Target& t,
                               uint64_t      handler_addr,
                               uint64_t      trampoline_addr,
                               IArchStubs&   stubs,
                               IRelocator&   relocator);

}
