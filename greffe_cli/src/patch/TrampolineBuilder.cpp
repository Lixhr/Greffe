#include "TrampolineBuilder.hpp"

#include <stdexcept>
#include <algorithm>
// Throws if the target instruction(s) cannot be safely overwritten by a branch.
// void TrampolineBuilder::validate_hook____(const Target& t) {


void TrampolineBuilder::create_branch(const Target& t) {
    std::vector<uint8_t> branch = t.stubs().branch(t.ea(), t.trampoline_addr());

    auto it = std::find_if(t.context().begin(), t.context().end(),                                                                                                                                
    [&t](const ContextEntry& c) { return c.ea == t.ea() ; }); 

    if (it == t.context().end())
        throw std::runtime_error("(Improbable) target instruction not found on context");

    size_t len = 0;
    while (it != t.context().end()) {
        len += it->raw.size();
        it ++;
    }
    throw std::runtime_error("(Improbable) target instruction not found on context");

    // std::vector<const ContextEntry &> reloc_instr;
    // size_t size = 0;
    // for (const auto& c : t.context()) {
    //     reloc_instr.push_back(c);
    //     size += 
    // }
}

std::vector<uint8_t> TrampolineBuilder::build(const Target&  /*t*/,
                                               uint64_t      /*handler_addr*/,
                                               uint64_t      /*trampoline_addr*/,
                                               IArchStubs&   /*stubs*/,
                                               IRelocator&   /*relocator*/) {
    throw std::runtime_error("TrampolineBuilder::build: not implemented");
}
