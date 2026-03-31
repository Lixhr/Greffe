#include "TrampolineBuilder.hpp"
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>

void TrampolineBuilder::branch_init(Target& t) {
    // generate a branch from the target to the trampoline
    IArchStubs &stubs = t.stubs();
    std::vector<uint8_t> branch = stubs.branch(t.ea(), t.trampoline_addr());

    // get our target
    auto it = std::find_if(t.context().begin(), t.context().end(),                                                                                                                                
    [&t](const ContextEntry& c) { return c.ea == t.ea() ; }); 

    if (it == t.context().end())
        throw std::runtime_error("Target instruction not found on context ??");


    size_t len = 0;
    std::vector<const ContextEntry *> relocd_instrs;

    while (it != t.context().end()) {
        // avoids overwriting instructions which is CODE XREFed
        // the first instr is ignored (func start)
        if (it->ea > t.ea() && it->is_xref_target)
            throw std::runtime_error("Patched branch overlaps a CODE XREF");

        len += it->raw.size();
        relocd_instrs.push_back(&(*it));
        it ++;
        
        if (len >= branch.size()) {
            t.setTrampolineRetAddr(it->ea);
            t.setRelocdInstrs(std::move(relocd_instrs));
            t.setBranchInstr(std::move(branch));
            return ;
        }
    }

    throw std::runtime_error("Patched branch overlaps end of function");
}

std::vector<uint8_t> TrampolineBuilder::build(const Target&  /*t*/,
                                               uint64_t      /*handler_addr*/,
                                               uint64_t      /*trampoline_addr*/,
                                               IArchStubs&   /*stubs*/,
                                               IRelocator&   /*relocator*/) {
    throw std::runtime_error("TrampolineBuilder::build: not implemented");
}
