#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"

PatchLayout::PatchLayout(const ProjectInfo& pinfo, TargetManager& targets) :
                         _pinfo(pinfo)
                       , _patch_plans(targets.plans())
                       , _patch_offset(pinfo.getPatchBase() - pinfo.getBinBase())
                        {}

void PatchLayout::set_trampoline_addr(PatchPlan* plan) {
    _patch_offset = plan->stubs->align_offset(_patch_offset);

    plan->trampoline_addr = _patch_offset + _pinfo.getPatchBase();
}

const SharedStub &PatchLayout::get_shstub(PatchPlan *plan) {
    auto it = std::find_if(_shstubs.begin(), _shstubs.end(),
        [&plan](const SharedStub& shstub) { 
            return (shstub.name() == plan->stubs->name()); 
        });

    if (it != _shstubs.end())
        return (*it);

    _shstubs.push_back(TrampolineBuilder::build_shstub(*plan));
    SharedStub &new_shstub = _shstubs.back();

    

    return (new_shstub);
}


void PatchLayout::create_patch_entry(PatchPlan *plan) {
    set_trampoline_addr(plan);
    TrampolineBuilder::branch_init(*plan);


}
