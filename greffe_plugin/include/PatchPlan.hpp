#pragma once

#include "ContextEntry.hpp"
#include "patch/arch/IArchStubs.hpp"
#include "patch/PatchLayoutEntry.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class PatchPlan : public PatchLayoutEntry {
    public:
        PatchPlan(std::string name, ea_t ea, ea_t end_ea, std::shared_ptr<IArchStubs> s)
            : PatchLayoutEntry(PLEType::entry_plan)
            , name(std::move(name)), target_ea(ea), target_end_ea(end_ea) { 
                stubs = std::move(s); 
            }

        std::string                         name;
        ea_t                                target_ea;
        ea_t                                target_end_ea;
        ea_t                                trampoline_ret_addr   = 0;
        std::vector<ContextEntry>           relocd_instr          = {};
        ea_t                                handler_ptr_addr = 0;
};