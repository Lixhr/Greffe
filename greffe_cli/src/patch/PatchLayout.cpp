#include "PatchLayout.hpp"
#include "TrampolineBuilder.hpp"

PatchLayout::PatchLayout(const ProjectInfo& pinfo, TargetManager& targets) :
                         _pinfo(pinfo)
                       , _patch_plans(targets.plans())
                       , _patch_offset(pinfo.getPatchBase() - pinfo.getBinBase())
                        {}


const std::vector<PatchPlan> & PatchLayout::patch_plans() const { return (_patch_plans);}

uint64_t PatchLayout::offset_to_addr(uint64_t offset) const {
    return (offset + _pinfo.getPatchBase());
}


void PatchLayout::set_trampoline_addr(PatchPlan* plan) {
    _patch_offset = plan->stubs->align_offset(_patch_offset);

    plan->trampoline_addr = _patch_offset + _pinfo.getPatchBase();
}

const SharedStub *PatchLayout::get_shstub(PatchPlan *plan) {
    auto it = std::find_if(_shstubs.begin(), _shstubs.end(),
        [&plan](const SharedStub& shstub) { 
            return (shstub.name() == plan->stubs->name()); 
        });

    if (it != _shstubs.end())
        return (&(*it));
    return (NULL);
}

const SharedStub *PatchLayout::create_shstub(PatchPlan *plan) {
    // Shared stub not created
    _shstubs.push_back(SharedStub(plan->stubs,
                                  _patch_offset, 
                                  _pinfo.getPatchBase() + _patch_offset));

    SharedStub &new_shstub = _shstubs.back();
    
    _patch_offset = new_shstub.end();
    return (&new_shstub);
}


void PatchLayout::create_patch_entry(PatchPlan *plan) {
    set_trampoline_addr(plan);
    TrampolineBuilder::branch_init(*plan);


    // get the shared-stubs
    const SharedStub *shstub = get_shstub(plan);
    if (!shstub)
        shstub = create_shstub(plan);


    // append the pre-trampolines
    TrampolineBuilder::init_trampoline(*plan, *shstub);

}
