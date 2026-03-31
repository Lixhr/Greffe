#include "TrampolineBuilder.hpp"

#include <stdexcept>


void TrampolineBuilder::validate(const Target& /*t*/, IArchStubs& /*stubs*/, IRelocator& /*relocator*/) {
    throw std::runtime_error("TrampolineBuilder::validate: not implemented");
}

std::vector<uint8_t> TrampolineBuilder::build(const Target&  /*t*/,
                                               uint64_t      /*handler_addr*/,
                                               uint64_t      /*trampoline_addr*/,
                                               IArchStubs&   /*stubs*/,
                                               IRelocator&   /*relocator*/) {
    throw std::runtime_error("TrampolineBuilder::build: not implemented");
}
