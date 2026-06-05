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
        PatchPlan(std::string name, std::string handler_name,
                  ea_t ea, ea_t end_ea, std::shared_ptr<IArchStubs> s)
            : PatchLayoutEntry(PLEType::entry_plan)
            , name(std::move(name))
            , handler_name(std::move(handler_name))
            , target_ea(ea), target_end_ea(end_ea) {
                stubs = std::move(s);
            }

        // Load from IDB
        PatchPlan(ea_t plan_addr, size_t plan_sz,
                  std::string name_,
                  std::string handler_name_,
                  ea_t target, ea_t target_end,
                  ea_t hptr_addr,
                  std::shared_ptr<IArchStubs> s);

        std::string                         name;
        std::string                         handler_name;
        ea_t                                target_ea;
        ea_t                                target_end_ea;
        ea_t                                trampoline_ret_addr   = 0;
        std::vector<ContextEntry>           relocd_instr          = {};
        ea_t                                handler_ptr_addr = 0;
        ea_t                                handler_addr = 0;
};