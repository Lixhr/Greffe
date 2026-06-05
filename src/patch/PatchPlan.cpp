#include "PatchPlan.hpp"
#include <bytes.hpp>

PatchPlan::PatchPlan(ea_t plan_addr, size_t plan_sz,
                     std::string name_,
                     std::string handler_name_,
                     ea_t target, ea_t target_end,
                     ea_t hptr_addr,
                     std::shared_ptr<IArchStubs> s)
    : PatchLayoutEntry(PLEType::entry_plan)
    , name(std::move(name_))
    , handler_name(std::move(handler_name_))
    , target_ea(target)
    , target_end_ea(target_end)
    , handler_ptr_addr(hptr_addr)
{
    stubs  = std::move(s);
    _addr  = plan_addr;
    _bytes.resize(plan_sz);
    get_bytes(_bytes.data(), plan_sz, plan_addr);
}
